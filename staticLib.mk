################OPTION###################
include ../../product_list_conf

COMPILEOPTION = -g -c -O3
ifeq ($(PLATFORM),HISI)
	LIB_DIR=hi_lib
else ifeq ($(PLATFORM), RK)
	LIB_DIR=rk-lib
else ifeq ($(PLATFORM), GPT) 
	LIB_DIR=gpt-lib
endif

#对应芯片音视频SDK所在目录
ifeq ($(HWMODEL), HI3519AV100)
COMPILEOPTION  += -DHI3519AV100
else ifeq ($(HWMODEL), HI3516DV300)
COMPILEOPTION  += -DHI3516DV300
endif

COMPILEOPTION += -DEPOLL
# COMPILEOPTION += -DHTTPS
COMPILEOPTION += -DHTTPD
COMPILEOPTION += -DDEVICEIO_SUPPORT
#COMPILEOPTION += -DPROFILE_G_SUPPORT
#COMPILEOPTION += -DPROFILE_C_SUPPORT
COMPILEOPTION += -DCREDENTIAL_SUPPORT
#COMPILEOPTION += -DACCESS_RULES
#COMPILEOPTION += -DSCHEDULE_SUPPORT
COMPILEOPTION += -DAUDIO_SUPPORT
COMPILEOPTION += -DMEDIA2_SUPPORT
COMPILEOPTION += -DPTZ_SUPPORT
#COMPILEOPTION += -DVIDEO_ANALYTICS
#COMPILEOPTION += -DTHERMAL_SUPPORT
#COMPILEOPTION += -DRECEIVER_SUPPORT
#COMPILEOPTION += -DIPFILTER_SUPPORT
#COMPILEOPTION += -DPROFILE_Q_SUPPORT
#COMPILEOPTION += -DLIBICAL

INCLUDEDIR = -I./bm -I./onvif -I./http
#INCLUDEDIR += -I./openssl/include -I./openssl/include/linux
INCLUDEDIR += -I../include
INCLUDEDIR += -I./camCtl
INCLUDEDIR += -I./camCtl/pelco_ptz 
INCLUDEDIR += -I./camCtl/visca

LINKOPTION = -g -o onvifserver
#LIBDIRS = -L./openssl/lib/linux -L./libical/lib/linux
OBJS = bm/word_analyse.o bm/util.o bm/sys_os.o bm/sys_log.o bm/sys_buf.o bm/ppstack.o bm/base64.o \
       bm/sha1.o bm/linked_list.o bm/hqueue.o bm/rfc_md5.o bm/xml_node.o bm/hxml.o http/http_srv.o \
       http/http_auth.o http/http_parse.o http/http_cln.o http/httpd.o onvif/soap.o onvif/onvif_probe.o \
       onvif/onvif_pkt.o onvif/onvif.o onvif/soap_parser.o onvif/onvif_doorcontrol.o onvif/onvif_device.o \
       onvif/onvif_timer.o onvif/onvif_event.o onvif/onvif_api.o onvif/onvif_ptz.o onvif/onvif_utils.o \
       onvif/onvif_media.o onvif/onvif_image.o onvif/onvif_cm.o onvif/onvif_recording.o onvif/onvif_analytics.o \
       onvif/onvif_cfg.o onvif/onvif_deviceio.o onvif/onvif_media2.o onvif/onvif_thermal.o \
       onvif/onvif_credential.o onvif/onvif_accessrules.o onvif/onvif_schedule.o onvif/onvif_receiver.o \
	   onvif_main.o	\
	  ./camCtl/pelco_ptz/ptz.o ./camCtl/ntp_conf.o \
	   ./camCtl/visca/libvisca.o ./camCtl/visca/libvisca_posix.o ./camCtl/visca/rw_config.o ./camCtl/visca/visca_api.o \
	   ./camCtl/set_config.o \
	   ./camCtl/cfg_file.o 
	#    main.o

STATIC_LIB =  libonvifserver.a
# OUTPUT = onvifserver

SHAREDLIB = -lpthread -ldl
#if define HTTPS
#SHAREDLIB += -lssl -lcrypto 
#if define LIBICAL
#SHAREDLIB += -lical -licalvcal

PROC_OPTION = DEFINE=_PROC_ MODE=ORACLE LINES=true CODE=CPP
ESQL_OPTION = -g
################OPTION END################
ESQL = esql
PROC = proc

# 编成静态库
$(STATIC_LIB): $(OBJS)
	$(AR) cr $@ $^
	cp -rf $(STATIC_LIB) ../$(LIB_DIR)


clean: 
	rm -f $(OBJS)
	rm -f $(STATIC_LIB)

.PHONY:clean all

all: $(STATIC_LIB)
.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

.cpp.o:
	$(CPP) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp
	
.cc.o:
	$(CPP) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp

.cxx.o:
	$(CPP) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp

.c.o:
	$(CC) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CC) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION)  $*.pc
