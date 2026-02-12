include $(SRCTREE)/applications/build/base.mk
all:
	chmod +x $(CURDIR)/bin -R
	python scripts/update_res.py all $(CURDIR) $(SRCTREE)/applications/$(CFG_PDT)/modules/ui/$(CFG_PDT_SUB)/$(UI_PACKET)

clean:

install:
