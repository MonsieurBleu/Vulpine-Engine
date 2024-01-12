ODIR=out_ktx
SDIR=out

SOURCES := $(wildcard $(SDIR)/*.png)
IMG := $(SOURCES:$(SDIR)/%.png=$(ODIR)/%.ktx)

install : $(IMG)

reinstall : clean install

out_ktx/%CE.ktx : out/%CE.png
	toktx $@ $< 

out_ktx/%NRM.ktx : out/%NRM.png
	toktx --assign_oetf linear $@ $< 