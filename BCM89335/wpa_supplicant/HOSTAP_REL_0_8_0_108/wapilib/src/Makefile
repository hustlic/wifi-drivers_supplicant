ifndef CC
CC=gcc
endif
ifndef AR
AR=ar
endif

OBJ_WAPI += cert.o ecc_openssl.o alg_comm.o sms4c.o wapi.o wapi_common.o wapi_interface.o wpi_pcrypt.o
SRC_WAPI += cert.c ecc_openssl.c alg_comm.c sms4c.c wapi.c wapi_common.c wapi_interface.c wpi_pcrypt.c

CFLAGS += $(CROSSTOOL_INCLUDE)
CFLAGS += -DLE -DECC_NO_ECC192_ECDH -DECC_NEED_NID_X9_62_PRIME192V4 -DASUE

LIB_WAPI = libwapi.a

all: $(OBJ_WAPI)
	$(AR) rcs $(LIB_WAPI) $(OBJ_WAPI)

OBJ_WAPI: $(SRC_WAPI)
	$(CC) $(CFLAGS) -c $(SRC_WAPI)

clean:
	rm -f *.o $(LIB_WAPI)
