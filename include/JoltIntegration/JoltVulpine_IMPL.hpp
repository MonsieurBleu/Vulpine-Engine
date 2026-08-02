#include <JoltIntegration/JoltIntegration.hpp>
#include <JoltIntegration/PhysicsCommons.hpp>

#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>

#include <Utils.hpp>

#include <cstdarg>

#define ENTITY_HEADER_SAFE_INCLUDE
#include <ECS/Entity.hpp>
#include <ECS/ComponentType3D.hpp>

std::mutex JoltVulpine::physicsMutex;
JPH::PhysicsSystem *JoltVulpine::jPhysicsSystem = nullptr;

std::mutex JoltVulpine::bodiesToAddMutex;
JPH::Array<JPH::BodyID> JoltVulpine::bodiesToAdd;

float JoltVulpine::currentPhysicFreq = 0.f;

LimitTimer JoltVulpine::physicsTicks;
BenchTimer JoltVulpine::physicsTimer = BenchTimer("Physics Timer");
BenchTimer JoltVulpine::physicsWorldUpdateTimer = BenchTimer("Physics World");
BenchTimer JoltVulpine::physicsSystemsTimer = BenchTimer("Physics Systems");

TickTimer JoltVulpine::physicInterpolationTick;
std::mutex JoltVulpine::physicInterpolationMutex;
bool JoltVulpine::doPhysicInterpolation = true;

bool JoltVulpine::useVelocityBasedMovement = false;
bool JoltVulpine::enablePhysics = false;

// JPH::AssertFailedFunction JPH::AssertFailed;

void JoltVulpine::TraceCallback(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	// cout << buffer << endl;
    WARNING_MESSAGE("Jolt Physics Callback : ", buffer);
}

bool JoltVulpine::AssertFailedCallback(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{
    ERROR_MESSAGE("Jolt Physics, Assert Failed ", inFile, ":", inLine, ": (", inExpression, ") ", inMessage != nullptr? inMessage : "")

	// Breakpoint
	return true;
}

JPH::BodyInterface & JoltVulpine::GetBodyInterface()
{
	return JoltVulpine::jPhysicsSystem->GetBodyInterface();
	
    /* If we are inside the jolt's main thread, no need to lock everything. */
    if(currentThreadID == JOLT_MAIN_THREAD_ID)
        return JoltVulpine::jPhysicsSystem->GetBodyInterfaceNoLock();
    else
        return JoltVulpine::jPhysicsSystem->GetBodyInterface();
}

template<> void Component<JoltBody>::ComponentElem::clean()
{
	JPH::BodyID *bodies = &data;
	auto &interface = JoltVulpine::jPhysicsSystem->GetBodyInterface();

	for(int i = 0; i < JOLT_VULPINE_MAX_ENTITY_SENSORS+1; i++)
	{
		if(!bodies[i].IsInvalid())
		{
			interface.RemoveBody(bodies[i]);
			interface.DestroyBody(bodies[i]);
			bodies[i] = JPH::BodyID();
		}
	}
}

DATA_READ_FUNC_ENTITY(JoltBody)
{
	DATA_READ_INIT(JoltBody)

	uint sensorCount = 0;
	bool hasMainBody = false;

	Vvec3 position = Vvec3(0);
	Vquat rotation = Vquat(JPH::Quat::sIdentity());

	if(e->has<State3D>())
	{
		position = e->comp<State3D>().position;
		rotation = e->comp<State3D>().rotation;
	}

	while(NEW_VALUE)
	{
		char* member = buff->read();
		bool sensorMode = false;
		JPH::StaticCompoundShapeSettings shapes;
		shapes.SetEmbedded();

		JPH::BodyCreationSettings settings;
		settings.mUserData = (uint64)e;

		uint collideCategory = 0;
		uint collideWIth = 0;

		IF_MEMBER(main)
		{
			hasMainBody = true;
			sensorMode = false;
			if(!data.IsInvalid())
				ERROR_MESSAGE("Redefining another main body for JoltBody. The other one will be overwritten!")
		}
		else
		IF_MEMBER(sensor)
		{
			sensorMode = true;
			settings.mIsSensor = true;
			settings.mMotionType = JPH::EMotionType::Kinematic;
			sensorCount ++;
			e->set<KynematicFlag>(hasMainBody && JoltVulpine::jPhysicsSystem->GetBodyInterface().GetMotionType(data) == JPH::EMotionType::Kinematic);
			if(sensorCount > JOLT_VULPINE_MAX_ENTITY_SENSORS)
			{
				sensorCount = JOLT_VULPINE_MAX_ENTITY_SENSORS;
				ERROR_MESSAGE("Maximum amount of sensor (", JOLT_VULPINE_MAX_ENTITY_SENSORS, ") reached, last sensor will be overwritten :(")
			}
		}
		else
		 	ERROR_MESSAGE("Unrecognized symbol '", member, "' for JoltBody. Excpected Main or Sensor");

		WHILE_NEW_VALUE
			IF_MEMBER_READ_VALUE(type)
			{
				if(!strcmp(value, "STATIC"))
					settings.mMotionType = JPH::EMotionType::Static;
				else if(!strcmp(value, "DYNAMIC"))
				{
					settings.mMotionType = JPH::EMotionType::Dynamic;
					e->set<DynamicState3D>(DynamicState3D({position, rotation}, {position, rotation}));
				}
				else if(!strcmp(value, "KYNEMATIC"))
				{
					e->set<KynematicFlag>(true);
					settings.mMotionType = JPH::EMotionType::Kinematic;
					e->set<DynamicState3D>(DynamicState3D({position, rotation}, {position, rotation}));
				}
				else
				{
					ERROR_MESSAGE("JoltBody type '", value, "' is not recognized. The body will be set to dynamic by default.")
				}
			}
			else 
			IF_MEMBER_READ_VALUE(mass)
			{
				if(settings.mOverrideMassProperties != JPH::EOverrideMassProperties::MassAndInertiaProvided)
					settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;

				settings.mMassPropertiesOverride.mMass = FastTextParser::read<float>(value);
			}
			else
			IF_MEMBER_READ_VALUE(inertia)
			{
				settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
				settings.mMassPropertiesOverride.mInertia = Vmat4(FastTextParser::read<glm::mat4>(value));
			}
			else 
			IF_MEMBER(angularLockX)
				settings.mAllowedDOFs ^= JPH::EAllowedDOFs::RotationX;
			else 
			IF_MEMBER(angularLockY)
				settings.mAllowedDOFs ^= JPH::EAllowedDOFs::RotationY;
			else 
			IF_MEMBER(angularLockZ)
				settings.mAllowedDOFs ^= JPH::EAllowedDOFs::RotationZ;
			else 
			IF_MEMBER_READ_VALUE(gravity)
			{
				settings.mGravityFactor = FastTextParser::read<bool>(value) ? 1.f : 0.f;
			}
			else 
			IF_MEMBER_READ_VALUE(friction)
			{
				settings.mFriction = FastTextParser::read<float>(value);
			}
			else 
			IF_MEMBER_READ_VALUE(bounciness)
			{
				settings.mRestitution = FastTextParser::read<float>(value);
			}
			else
			IF_MEMBER(CollisionCategory)
			{
				while(NEW_VALUE)
				{
					uint category = 64;
					MAP_SAFE_READ(JoltVulpine::Layers::CategoriesMap, buff, category, buff->read());
					collideCategory |= 1<<category;
				}
			}
			else
			IF_MEMBER(CollideWith)
			{
				while(NEW_VALUE)
				{
					uint category = 64;
					MAP_SAFE_READ(JoltVulpine::Layers::CategoriesMap, buff, category, buff->read());
					collideWIth |= 1<<category;
				}
			}
			else 
			IF_MEMBER(Colliders)
			{
				while(NEW_VALUE)
				{
					member = buff->read();
					IF_MEMBER(Collider)
					{
						JPH::CompoundShapeSettings::SubShapeSettings collider;
						collider.mPosition = Jvec3::sZero();
						collider.mRotation = Jquat::sIdentity();

						WHILE_NEW_VALUE
							/******** 
								Loading collider shape
							********/
							IF_MEMBER(CapsuleShape)
							{
								auto capsule = new JPH::CapsuleShapeSettings();

								WHILE_NEW_VALUE
									IF_MEMBER_READ_VALUE(height)
										capsule->mHalfHeightOfCylinder = FastTextParser::read<float>(value);
									else 
									IF_MEMBER_READ_VALUE(radius)
										capsule->mRadius = FastTextParser::read<float>(value);
									else
									IF_MEMBER_READ_VALUE(density)
										capsule->mDensity = FastTextParser::read<float>(value);
								WHILE_NEW_VALUE_END

								collider.mShape = capsule;
							}
							else 
							IF_MEMBER(CylinderShape)
							{
								auto cylinder = new JPH::CylinderShapeSettings();

								WHILE_NEW_VALUE
									IF_MEMBER_READ_VALUE(height)
										cylinder->mHalfHeight = FastTextParser::read<float>(value);
									else
									IF_MEMBER_READ_VALUE(radius)
										cylinder->mRadius = FastTextParser::read<float>(value);
									else
									IF_MEMBER_READ_VALUE(density)
										cylinder->mDensity = FastTextParser::read<float>(value);
								WHILE_NEW_VALUE_END

								collider.mShape = cylinder;
							}
							else
							IF_MEMBER(BoxShape)
							{
								auto box = new JPH::BoxShapeSettings();
								WHILE_NEW_VALUE
									IF_MEMBER_READ_VALUE(extent)
										box->mHalfExtent = Vvec3(FastTextParser::read<glm::vec3>(value));
									else
									IF_MEMBER_READ_VALUE(density)
										box->mDensity = FastTextParser::read<float>(value);
								WHILE_NEW_VALUE_END
								collider.mShape = box;
							}
							else
							IF_MEMBER(SphereShape)
							{
								auto sphere = new JPH::SphereShapeSettings();
								WHILE_NEW_VALUE
									IF_MEMBER_READ_VALUE(radius)
										sphere->mRadius = FastTextParser::read<float>(value);
									else
									IF_MEMBER_READ_VALUE(density)
										sphere->mDensity = FastTextParser::read<float>(value);
								WHILE_NEW_VALUE_END
								collider.mShape = sphere;
							}
							else
							IF_MEMBER(ConvexMeshShape)
							{
								auto mesh = new JPH::ConvexHullShapeSettings;
								WHILE_NEW_VALUE
									IF_MEMBER_READ_VALUE(size)
										mesh->mPoints.reserve(FastTextParser::read<uint>(value));
									else
									IF_MEMBER_READ_VALUE(vertex)
										mesh->mPoints.push_back(Vvec3(FastTextParser::read<glm::vec3>(value)));
									else
									IF_MEMBER_READ_VALUE(density)
										mesh->mDensity = FastTextParser::read<float>(value);
								WHILE_NEW_VALUE_END

								collider.mShape = mesh;
							}
							/******** 
								Loading common collider informations
							********/
							else					
							IF_MEMBER(Transform)
							{
								WHILE_NEW_VALUE
									IF_MEMBER_READ_VALUE(position)
										collider.mPosition = Vvec3(FastTextParser::read<glm::vec3>(value));
									else
									IF_MEMBER_READ_VALUE(quaternion)
										collider.mRotation = Vquat(FastTextParser::read<glm::quat>(value));
								WHILE_NEW_VALUE_END
							}
						WHILE_NEW_VALUE_END

						shapes.mSubShapes.push_back(collider);
						collider.mUserData = (uint64)e;
					}
					else 
						ERROR_MESSAGE("Expected a collider inside colliders list, but got symbol '", member, "'")
				}

				shapes.mUserData = (uint64)e;

				/******** 
					Adding all colliders on body
				********/
				JPH::Shape::ShapeResult result = shapes.Create();

				if(result.IsValid())
					settings.SetShape(result.Get());
				else
				{
					ERROR_MESSAGE("Non-valid shape ", result.GetError())
				}
			}
		WHILE_NEW_VALUE_END
			
		/******** 
			Creating the body
		********/
		settings.mObjectLayer = JPH::ObjectLayerPairFilterMask::sGetObjectLayer(collideCategory, collideWIth);
		settings.mPosition = position;
		settings.mRotation = rotation;
		JPH::Body *body = JoltVulpine::jPhysicsSystem->GetBodyInterface().CreateBody(settings);

		// settings.mMotionQuality = JPH::EMotionQuality::LinearCast;

		if(sensorMode) 
			data.sensors[sensorCount-1] = body->GetID();
		else
		 	data = JoltBody(body->GetID());
			
		JoltVulpine::bodiesToAddMutex.lock();
		JoltVulpine::bodiesToAdd.push_back(body->GetID());
		JoltVulpine::bodiesToAddMutex.unlock();
	}

	/******** 
		Adding all created bodies
	********/
	// auto ptr = hasMainBody ? &data : data.sensors;
	// auto size = sensorCount + hasMainBody;
	// auto addstate = JoltVulpine::jPhysicsSystem->GetBodyInterface().AddBodiesPrepare(ptr, size);
	// JoltVulpine::jPhysicsSystem->GetBodyInterface().AddBodiesFinalize(ptr, size, addstate, JPH::EActivation::Activate);

	DATA_READ_END
}

DATA_WRITE_FUNC_INIT(JoltBody)
	
	JPH::BodyID *ids = (JPH::BodyID*)&data;

	for(int i = 0; i < JOLT_VULPINE_MAX_ENTITY_SENSORS; i++)
	{
		auto &body = ids[i];

		if(body.IsInvalid()) continue;

		auto &interface = JoltVulpine::jPhysicsSystem->GetBodyInterface();
		const JPH::Body* const body2 = JoltVulpine::jPhysicsSystem->GetBodyLockInterface().TryGetBody(body);
		const auto &settings = body2->GetBodyCreationSettings();

		out->Entry();

		if(i)
			WRITE_NAME(sensor, out)
		else
			WRITE_NAME(main, out)
		
		out->Tabulate();

			auto shape = interface.GetShape(body);

			if(!i)
			{
				out->Entry();
				WRITE_NAME(type, out)
				switch (interface.GetMotionType(body)) {
					case JPH::EMotionType::Dynamic   : WRITE_NAME("DYNAMIC", out);   break;
					case JPH::EMotionType::Kinematic : WRITE_NAME("KYNEMATIC", out); break;
					case JPH::EMotionType::Static    : WRITE_NAME("STATIC", out);    break;
				}

				switch (settings.mOverrideMassProperties) {
					case JPH::EOverrideMassProperties::CalculateInertia : 
						out->Entry(); WRITE_NAME(mass, out)
						FastTextParser::write<float>(shape->GetMassProperties().mMass, out->getReadHead());
					break;

					case JPH::EOverrideMassProperties::MassAndInertiaProvided :
						out->Entry(); WRITE_NAME(mass, out)
						FastTextParser::write<float>(shape->GetMassProperties().mMass, out->getReadHead());

						// out->Entry(); WRITE_NAME(inertia, out)
						// FastTextParser::write<glm::mat4>(Vmat4(shape->GetMassProperties().mInertia), out->getReadHead());
					break;

					case JPH::EOverrideMassProperties::CalculateMassAndInertia : 

					break;
				};
			}
			
			if((settings.mAllowedDOFs & JPH::EAllowedDOFs::RotationX) == JPH::EAllowedDOFs::None)
			{
				out->Entry(); WRITE_NAME(angularLockX, out);
			}
			if((settings.mAllowedDOFs & JPH::EAllowedDOFs::RotationY) == JPH::EAllowedDOFs::None)
			{
				out->Entry(); WRITE_NAME(angularLockY, out);
			}
			if((settings.mAllowedDOFs & JPH::EAllowedDOFs::RotationZ) == JPH::EAllowedDOFs::None)
			{
				out->Entry(); WRITE_NAME(angularLockZ, out);
			}

			if(!i)
			{
				out->Entry(); WRITE_NAME(gravity, out);
				if(interface.GetGravityFactor(body) > 0.f) 
					WRITE_NAME(t, out)
				else
					 WRITE_NAME(f, out)
	
				float friction = interface.GetFriction(body);
				if(friction != 0.f)
				{
					out->Entry(); WRITE_NAME(friction, out);
					FastTextParser::write<float>(friction, out->getReadHead());
				}
	
				float bounciness = interface.GetRestitution(body);
				if(bounciness != 0.f)
				{
					out->Entry(); WRITE_NAME(bounciness, out);
					FastTextParser::write<float>(bounciness, out->getReadHead());
				}
			}

			auto category = interface.GetObjectLayer(body);
			uint collision = JPH::ObjectLayerPairFilterMask::sGetGroup(category);
			uint collideWith = JPH::ObjectLayerPairFilterMask::sGetMask(category);

			out->Entry(); WRITE_NAME(CollisionCategory, out);
			out->Tabulate();
			for(auto &i : JoltVulpine::Layers::CategoriesMap)
				if(collision & 1<<i.second){out->Entry(); out->write(i.first.c_str(), i.first.size());}
			out->Break();

			out->Entry(); WRITE_NAME(CollideWith, out);
			out->Tabulate();
			for(auto &i : JoltVulpine::Layers::CategoriesMap)
				if(collideWith & 1<<i.second){out->Entry(); out->write(i.first.c_str(), i.first.size());}
			out->Break();

			out->Entry(); WRITE_NAME(Colliders, out);
			out->Tabulate();

			bool isCompond = shape->GetType() == JPH::EShapeType::Compound; 
			bool isConvexSHape = false;

			if(!isCompond && shape->GetSubType() != JPH::EShapeSubType::RotatedTranslated)
			{
				// ERROR_MESSAGE("Vulpine only support saving bodies with compound or rotated translated shape type. Current typeID is ", (int)shape->GetType());
				
				isConvexSHape = true;
			}
			// else
			{
				auto shapeList = isCompond ? 
					((JPH::StaticCompoundShape *)shape.GetPtr())->GetSubShapes() 
					: 
					JPH::CompoundShape::SubShapes({JPH::CompoundShape::SubShape()})
				;

				if(!isCompond)
				{
					if(!isConvexSHape)
					{
						auto rt = (JPH::RotatedTranslatedShape*)shape.GetPtr();
						shapeList.front().mShape = rt->GetInnerShape();
						rt->GetPosition().StoreFloat3(&shapeList.front().mPositionCOM);
						rt->GetRotation().StoreFloat3(&shapeList.front().mRotation);
						shapeList.front().mShape = rt->GetInnerShape();
					}
					else
					{
						shapeList.front().mShape = shape;
					}
				}


				for(auto &s : shapeList)
				{
					out->Entry(); WRITE_NAME(Collider, out)
					out->Tabulate();
					switch (s.mShape->GetSubType()) {
						case JPH::EShapeSubType::Sphere :
						{
							auto sphere = (JPH::SphereShape *)s.mShape.GetPtr();

							out->Entry(); WRITE_NAME(SphereShape, out)
							out->Tabulate();
								out->Entry(); WRITE_NAME(radius, out)
								FastTextParser::write<float>(sphere->GetRadius(), out->getReadHead());

								if(sphere->GetDensity() != 1000.f)
								{
									out->Entry(); WRITE_NAME(density, out)
									FastTextParser::write<float>(sphere->GetDensity(), out->getReadHead());
								}
							out->Break();
						}
						break;

						case JPH::EShapeSubType::Capsule :
						{
							auto capsule = (JPH::CapsuleShape *)s.mShape.GetPtr();

							out->Entry(); WRITE_NAME(CapsuleShape, out)
							out->Tabulate();
								out->Entry(); WRITE_NAME(radius, out)
								FastTextParser::write<float>(capsule->GetRadius(), out->getReadHead());

								out->Entry(); WRITE_NAME(height, out)
								FastTextParser::write<float>(capsule->GetHalfHeightOfCylinder(), out->getReadHead());

								if(capsule->GetDensity() != 1000.f)
								{
									out->Entry(); WRITE_NAME(density, out)
									FastTextParser::write<float>(capsule->GetDensity(), out->getReadHead());
								}
							out->Break();
						}
						break;

						case JPH::EShapeSubType::Cylinder :
						{
							auto cylinder = (JPH::CylinderShape *)s.mShape.GetPtr();

							out->Entry(); WRITE_NAME(CylinderShape, out)
							out->Tabulate();
								out->Entry(); WRITE_NAME(radius, out)
								FastTextParser::write<float>(cylinder->GetRadius(), out->getReadHead());

								out->Entry(); WRITE_NAME(height, out)
								FastTextParser::write<float>(cylinder->GetHalfHeight(), out->getReadHead());

								if(cylinder->GetDensity() != 1000.f)
								{
									out->Entry(); WRITE_NAME(density, out)
									FastTextParser::write<float>(cylinder->GetDensity(), out->getReadHead());
								}
							out->Break();
						}
						break;

						case JPH::EShapeSubType::Box :
						{
							auto box = (JPH::BoxShape *)s.mShape.GetPtr();

							out->Entry(); WRITE_NAME(BoxShape, out)
							out->Tabulate();
								out->Entry(); WRITE_NAME(extent, out)
								FastTextParser::write<glm::vec3>(Vvec3(box->GetHalfExtent()), out->getReadHead());

								if(box->GetDensity() != 1000.f)
								{
									out->Entry(); WRITE_NAME(density, out)
									FastTextParser::write<float>(box->GetDensity(), out->getReadHead());
								}
							out->Break();
						}
						break;

						case JPH::EShapeSubType::ConvexHull :
						{
							auto mesh = (JPH::ConvexHullShape *)s.mShape.GetPtr();

							out->Entry(); WRITE_NAME(ConvexMeshShape, out)
							out->Tabulate();
							
								if(mesh->GetDensity() != 1000.f)
								{
									out->Entry(); WRITE_NAME(density, out)
									FastTextParser::write<uint>(mesh->GetNumPoints(), out->getReadHead());
								}

								out->Entry(); WRITE_NAME(size, out)
								FastTextParser::write<uint>(mesh->GetNumPoints(), out->getReadHead());

								for(int j = 0; j < mesh->GetNumPoints(); j++)
								{
									out->Entry(); WRITE_NAME(vertex, out)
									FastTextParser::write<glm::vec3>(Vvec3(mesh->GetPoint(j)), out->getReadHead());
								}
							out->Break();
						}
						break;


						default:
							ERROR_MESSAGE("Can't save unsuported shape type ", (int)s.mShape->GetSubType(), ". Only sphere, cylinder, box, capsule and convex hull are supported.")
						break;
					}
					
					out->Entry(); WRITE_NAME(Transform, out)
					out->Tabulate();

						Vvec3 position = isCompond ? s.GetPositionCOM() + body2->GetCenterOfMassPosition() - body2->GetPosition(): s.GetPositionCOM();

						out->Entry(); WRITE_NAME(position, out)
						FastTextParser::write<glm::vec3>(position, out->getReadHead());

						Jmat4 m = s.GetLocalTransformNoScale(Vvec3(1, 1, 1));
						Jvec3 test = m * s.GetPositionCOM();
						
						if(!s.mIsRotationIdentity)
						{
							out->Entry(); WRITE_NAME(quaternion, out);
							FastTextParser::write<glm::quat>(Vquat(s.GetRotation()), out->getReadHead());
						}
						
					out->Break();
					out->Break();
				}
			}

			out->Break();
		out->Break();
	}

DATA_WRITE_END_FUNC