PROGRAM_ppu	= knn 

INCLUDE = -I ../include/
IMPORTS = ../spu/lib_knn_spu.a -lspe2 -lpthread

ifdef CELL_TOP
	include $(CELL_TOP)/buildutils/make.footer
else
	include /opt/cell/sdk/buildutils/make.footer
endif