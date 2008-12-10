PROGRAM_ppu	= knn_ppu 

INCLUDE = -I ../include/
IMPORTS = ../spu/lib_knn_spu.a -lspe2 -lpthread -lmisc
CC_OPT_LEVEL = -g3

ifdef CELL_TOP
	include $(CELL_TOP)/buildutils/make.footer
else
	include /opt/cell/sdk/buildutils/make.footer
endif
