ODIR=out_ktx
SDIR=out

SOURCES := $(wildcard $(SDIR)/*.png)
IMG := $(SOURCES:$(SDIR)/%.png=$(ODIR)/%.ktx)

# KTX_FLAGS := --genmipmap 
KTX_FLAGS := --generate-mipmap 

default : $(IMG)

out_ktx/%CE.ktx : out/%CE.png
out_ktx/CE.ktx : out/CE.png
	ktx create $(KTX_FLAGS) --assign-oetf srgb --format R8G8B8A8_SRGB $< $@
# toktx $(KTX_FLAGS) $@ $< 

out_ktx/%NRM.ktx : out/%NRM.png
out_ktx/NRM.ktx : out/NRM.png
	ktx create $(KTX_FLAGS) --assign-oetf linear --format R8G8B8A8_UNORM $< $@
# 	toktx --assign_oetf linear $(KTX_FLAGS) $@ $< 

redo : clean install

install : 
	g++ -Wall -O3 convertor.cpp -o convertor.exe

