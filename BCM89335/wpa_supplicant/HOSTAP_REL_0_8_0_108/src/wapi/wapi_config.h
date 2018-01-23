/** @file wapi_config.h
 *  @brief This header file contains data structures and function declarations of wapi_config
 *
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */

#ifndef __WAPI_CONFIG_H__
#define __WAPI_CONFIG_H__

#include "common.h"

#define MAX_SSID_LEN 32
#define BK_LEN 16
#define EAP_PSK_LEN 16
#define EAP_PSK_LEN 16
#define WAPI_CIPHER_NONE BIT(0)
#define WAPI_CIPHER_SMS4 BIT(4)

#define WAPI_KEY_MGMT_NONE 0
#define WAPI_KEY_MGMT_CERT BIT(1)
#define WAPI_KEY_MGMT_PSK BIT(2)

/* Separators */
#define SEP_EQUAL '='
#define SEP_SPACE ' '
#define SEP_TAB (char)0x9

#define ENABLE 		1
#define DISABLE 		2
#define MYVERIFY 		3
#define LENGTH		255
#define KEYS_MAX 	255


struct prop_data_st
{
  char *key;
  char *value;
};

typedef struct prop_data_st prop_data;

struct asue_config
{
	char cert_name[256];
	unsigned short used_cert;
	unsigned short pad;
};

struct _asue_cert_info {
	struct asue_config config;
};

int load_prop (char type, char *file_name, prop_data properties[]);
char *get_prop (char *key, char *result, prop_data properties[], int count);
int save_global_conf(char type, char* filename, char *command, char *key, char *value);
int free_prop (prop_data properties[], int count);
int hex2int(char c);
void *get_buffer(int len);
void *free_buffer(void *buffer, int len);
int str2byte( unsigned char *str, int len,  char *byte_out);

struct wapi_config{
	u8 *ssid;
	size_t ssid_len;
	
	u8 *psk;
	size_t psk_len;
	u8 psk_bk[BK_LEN];
	int psk_set;
	
	int wapi_policy;
	int wpi_policy;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;

	int proto;
	int auth_alg;
	char *cert_name;
	int disabled;
};
void wapi_config_free(struct wapi_config *config);
struct wapi_config * wapi_config_read(const char *config_file);
int change_cert_format(const  char *cert_file, unsigned char *out_user, int len_user, unsigned char *out_asu, int len_asu);
#endif	
