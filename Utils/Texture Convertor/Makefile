ODIR=out_ktx
SDIR=out

SOURCES := $(wildcard $(SDIR)/*.png)
IMG := $(SOURCES:$(SDIR)/%.png=$(ODIR)/%.ktx)

KTX_FLAGS := --genmipmap 

default : $(IMG)

out_ktx/%CE.ktx : out/%CE.png
out_ktx/CE.ktx : out/CE.png
	toktx $(KTX_FLAGS) $@ $< 

out_ktx/%NRM.ktx : out/%NRM.png
out_ktx/NRM.ktx : out/NRM.png
	toktx --assign_oetf linear $(KTX_FLAGS) $@ $< 

redo : clean install

install : 
	g++ -Wall -O3 convertor.cpp -o convertor.exe

