/** @file wapi_config.c
  * @brief This file contains functions for wapi_config
 *
 *  Copyright (C) 2001-2008, Iwncomm Ltd.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

//#include "wapi_common.h"
#include "wapi_config.h"
#include "config.h"
#include "wapi.h"
#include "utils/eloop.h"
#include "wapi/wapi_interface.h"
#include "wapi_asue.h"
#include "wpa_supplicant_i.h"

#define SEP_Q_MARK	'"'

static int get_pr (char type, char *line, prop_data *prop)
{
    char *index = NULL;	
    char *keyptr = NULL;

    if (line[0] != '#')
    {
        // modified by kWlW, separator changed to this  vvv  from "="
        keyptr = (char *)strstr(line, "setenv ");	
		//finds the first occurrence of the substring 'setenv' in line
        if (keyptr != NULL) line = keyptr + 7;   
		/* Add size of 'setenv ' string */
		index =  strstr (line, type == SEP_EQUAL ? "=" : type == SEP_SPACE ? " " : "\t");	
        if (((char *)index) != NULL)
        {
            *((char *) index) = '\0';
			/* skip leading spaces or tabs at the beginning of the line */
			while ( (*line == ' ') || (*line == '\t') ){
				line++;
			}
            if ((prop->key = (char *)malloc (strlen (line) + 1)) != NULL &&
                (prop->value =(char *) malloc (strlen ((char *) (index + 1)) + 1)) != NULL)
            {
                strcpy (prop->key, line);
                strcpy (prop->value, (char *) (index + 1));
                return 1;
            }
        }
    }
    return 0;
}


int load_prop (char type, char *file_name, prop_data properties[])
{
	FILE *fid;
	char line[1024];
	int prop_count = 0;
    char * buf;

	if ((fid = fopen (file_name, "r")) == NULL)    return 0;
	while (!feof (fid))
	{
		buf = fgets(line, 255, fid);
		if(feof(fid)) break;
		if (strchr(line, '\n'))  *(strchr(line, '\n')) = '\0';
		if (get_pr (type, line, (prop_data *) & properties[prop_count]))
		{
			prop_count++;
		}
	}
	fclose (fid);
	return prop_count;
}

static char *strip (char *string, char delimiter)
{
  register int x, y;

  y = strlen (string);
  x = 0;
  while ((x < y) && (string[x] == delimiter))
    {
	  x++;
    };
  while ((y > 0) && (string[y - 1] == delimiter))
    {
      y--;
    };
  string[y] = 0;
  return string + x;
}
char *get_prop (char *key, char *result, prop_data properties[], int count)
{
  int index;
  char tmp_str[256] = "";
  for (index = 0; index < count; index++)
    {
      if (strcmp (properties[index].key, key) == 0)
	{
	  strcpy(tmp_str, properties[index].value);	
	  strcpy (result, strip(tmp_str, SEP_Q_MARK));
	  return result;
	}
    }
  return NULL;
}

int hex2int(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return -1;
}

int str2byte( unsigned char *str, int len,  char *byte_out)
{
	int i, val, val2;
	unsigned char *pos = str;
	
	for (i = 0; i < len/2; i++) {
		val = hex2int(*pos++);
		if (val < 0)
			return -1;
		val2 = hex2int(*pos++);
		if (val2 < 0)
			return -1;
		byte_out[i] = (val * 16 + val2) & 0xff;
	}

	return 0;
}

void *get_buffer(int len)
{
	char *buffer=NULL;
	buffer = (char *)malloc(len);
	if(buffer)
		memset(buffer, 0, len);
	else
		buffer = NULL;
	return buffer;
}
void *free_buffer(void *buffer, int len)
{
	char *tmpbuf = (char *)buffer;

	if(tmpbuf != NULL)
	{
		memset(tmpbuf, 0, len);
		free(tmpbuf);
		return NULL;
	}
	else
		return NULL;
}

int free_prop (prop_data properties[], int count)
{
  int index;
  for (index = 0; index < count; index++)
    {
      free (properties[index].key);
      free (properties[index].value);
    }
  return 1;
}

int save_global_conf(char type, char* filename, char *command, char *key, char *value)
{
	FILE* fidIn, *fidOut;
	char line[1024], tmp_line[1024];
        char tmp_fname[255], *index;
        char separator[2];
        int i=0;
    char * buf;
	
	if ((fidIn=fopen(filename,"r"))==NULL) return 0;
	strcpy(tmp_fname,filename);	
	strcat(tmp_fname,".new");
	
        if ((fidOut=fopen(tmp_fname,"w"))==NULL) return 0;

        strcpy(separator,(type==SEP_EQUAL?"=":type==SEP_SPACE?" ":"\t"));
        type=separator[0];

	while(!feof(fidIn))
	{
		buf = fgets(line,1024,fidIn);
		strcpy(tmp_line,command);
		strcat(tmp_line,key);
                if (((index=strstr(line,tmp_line))!=NULL) && (index==line)) {
					printf("finding\n");
                    i=1;
                    if (!feof(fidIn)) {
                        if (type != '0')
                            fprintf(fidOut,"%s%s%c%s\n",command,key,type,value);
                        else
                            fprintf(fidOut,"%s%s%s\n",command,key,value);
                    }
                } else
                    if (!feof(fidIn)) fprintf(fidOut,"%s",line);
        }

        if (i==0) {
            if (type != '0') { fprintf(fidOut,"%s%s%c%s\n",command,key,type,value); }
            else fprintf(fidOut,"%s%s%s\n",command,key,value);
        }
             
	fclose(fidIn);
	fclose(fidOut);
	unlink(filename);
	rename(tmp_fname,filename);
	return 1;

}

int change_cert_format(const  char *cert_file, 
		       unsigned char *out_user, 
		       int len_user,
		       unsigned char *out_asu,
		       int len_asu);


void wapi_config_free(struct wapi_config *config)
{
	struct wapi_config *prev = NULL;
	prev = config;
	if (prev) {
		if(prev->ssid)
			free(prev->ssid);
		if(prev->cert_name)
			free(prev->cert_name);
		free(config);
	}
}

/**
  * Save User certificate status
*/
int save_cert_status(char *fileconfig, char *cert_flag)
{
	int res = 0;
	res = !save_global_conf(SEP_EQUAL,fileconfig,"",  "CERT_STATUS",cert_flag);
	if(res != 0)
	{
		wpa_printf(MSG_DEBUG, "open file %s error\n", fileconfig);
	}
	return res;
}


struct wapi_config * wapi_config_read(const char *config_file)
{
	char buf[256] = {0,};
	prop_data properties[KEYS_MAX];
	int prop_count=0;
	struct wapi_config *config = NULL;
	CNTAP_PARA lib_param;  
	u8 psk_type = 0;
    struct wpa_global *eloop = (struct wpa_global *)eloop_get_user_data();
	
    if ( eloop == NULL ) return NULL;

	config = malloc(sizeof(*config));
	if (config == NULL)
		return NULL;
	memset(config, 0, sizeof(*config));

	wpa_printf(MSG_DEBUG, "Reading configuration file '%s'",
		   config_file);
	prop_count=load_prop(SEP_EQUAL, (char *)config_file,properties);	

	if(prop_count == 0){
		wpa_printf(MSG_ERROR, "no keys and values pair in configuration file '%s'",
			   config_file);
		return NULL;
	}

	bzero(buf, 256);
	
	/*Get Authentication and Key Management Suites*/
	get_prop("wapi", buf, properties, prop_count);
	config->wapi_policy = atoi(buf);
	wpa_printf(MSG_DEBUG, "wapi_policy '%d'",   config->wapi_policy);
	
	if (config->wapi_policy & 0x08)
	{
		lib_param.authType = AUTH_TYPE_WAPI;
	}
	else
		if (config->wapi_policy & 0x04)
		{
			lib_param.authType = AUTH_TYPE_WAPI_PSK;
		}
		else
		{
			lib_param.authType = AUTH_TYPE_NONE_WAPI;
		}
	
	wpa_printf(MSG_DEBUG, "WAPI '%d'", config->wapi_policy);

	if(config->wapi_policy != 0)
	{
		config->group_cipher = WAPI_CIPHER_SMS4;
		config->pairwise_cipher = WAPI_CIPHER_SMS4;
	}
	bzero(buf, 256);

	get_prop("ssid", buf, properties, prop_count);
	config->ssid_len = strlen(buf);
	config->ssid = (u8*)strdup(buf);
	wpa_hexdump_ascii(MSG_DEBUG, " SSID:", config->ssid, config->ssid_len);
	
	bzero(buf, 256);
	get_prop("cert_index", buf, properties, prop_count);
	eloop->cert_info.config.used_cert = (u8)atoi(buf);/*1:x509 2:GBW */

	bzero(buf, 256);
	get_prop("cert_name", eloop->cert_info.config.cert_name, properties, prop_count);
	wpa_printf(MSG_DEBUG, "CERT_NAME '%s'", eloop->cert_info.config.cert_name);

	if((config->wapi_policy & 0x08) && strlen(eloop->cert_info.config.cert_name))
	{		

		change_cert_format(eloop->cert_info.config.cert_name, 
							lib_param.para.user, 2048,
							lib_param.para.as, 2048);
	}
	
	memset(buf, 0, sizeof(buf));
	get_prop("psk_key_type", buf, properties, prop_count);
	psk_type = (u8)atoi(buf);
	wpa_printf(MSG_DEBUG, "PSK_KEY_TYPE '%d'",   psk_type);

	bzero(buf, 256);
	get_prop("psk", buf, properties, prop_count);
	wpa_printf(MSG_DEBUG, "PSK'%s'", buf);
	
	if(config->wapi_policy & WAPI_KEY_MGMT_PSK){ /*bit 2:PSK*/
		
		lib_param.para.kt = psk_type;
		lib_param.para.kl = strlen(buf);
		memcpy(lib_param.para.kv, buf, lib_param.para.kl);
	}

	WAI_CNTAPPARA_SET(&lib_param);	// do it after the wpa_s has been attached to global
	
	/* copy wapi_config stuff into wpa_config structure? */
	free_prop(properties,prop_count);
	return config;
}



/**
 * find mark in src
 */
static const unsigned char* findmark_mem(const unsigned char* src, int lsrc, const unsigned char* mark, int lmark)
{
	const unsigned char* p = src;
	const unsigned char* pe = src+lsrc;
	if (NULL==src || NULL==mark || lsrc<0 || lmark<0 || lsrc<lmark)
	{
		return NULL;
	}
	pe -= lmark;
	for (; p<=pe; p++)
	{
		if (0 == memcmp(p, mark, lmark))
		{
			return p;
		}
	}
	return NULL;
}

static const unsigned char* findstrmark_mem(const unsigned char* src, int lsrc, const char* mark)
{
	return findmark_mem(src, lsrc, (const unsigned char*)mark, strlen(mark));
}

int change_cert_format(const  char *cert_file, 
						unsigned char *out_user, 
						int len_user,
						unsigned char *out_asu,
						int len_asu)
{
	int ret = -1;
	int len = 0;
	unsigned char buf[1024*8] = {0};

	const char* mark_asu_s = "-----BEGIN ASU CERTIFICATE-----";
	const char* mark_asu_e = "-----END ASU CERTIFICATE-----";
	const char* mark_user_s = "-----BEGIN USER CERTIFICATE-----";
	const char* mark_user_e = "-----END USER CERTIFICATE-----";
	const char* mark_ec_s = "-----BEGIN EC PRIVATE KEY-----";
	const char* mark_ec_e = "-----END EC PRIVATE KEY-----";
	const char* mark_s = "-----BEGIN CERTIFICATE-----";
	const char* mark_e = "-----END CERTIFICATE-----";

	if (cert_file == NULL || out_user == NULL || out_asu == NULL || len_user <= 0 || len_asu <= 0)
	{
		return ret;
	}

	{
		FILE* fp = fopen((const char*)cert_file, "rb");
		if (fp == NULL)
		{
			return ret;
		}
		len = fread(buf, 1, 1024*8, fp);
		fclose(fp);
		if (len <= 0)
		{
			return ret;
		}
	}

	{
		unsigned char *p  = NULL;
		const unsigned char *ps  = NULL;
		const unsigned char *pe  = NULL;
		ps = findstrmark_mem(buf, len, mark_asu_s);
		pe = findstrmark_mem(buf, len, mark_asu_e);
		if (NULL==ps || NULL==pe || ps>=pe)
		{
			return ret;
		}
		if ((unsigned int)len_asu < strlen(mark_s)+pe-ps-strlen(mark_asu_s)+strlen(mark_e))
		{
			return ret;
		}
		p = out_asu;
		memcpy(p, mark_s, strlen(mark_s));
		p += strlen(mark_s);
		memcpy(p, ps+strlen(mark_asu_s), pe-ps-strlen(mark_asu_s));
		p += pe-ps-strlen(mark_asu_s);
		memcpy(p, mark_e, strlen(mark_e));

		ps = findstrmark_mem(buf, len, mark_user_s);
		pe = findstrmark_mem(buf, len, mark_user_e);
		if (NULL==ps || NULL==pe || ps>=pe)
		{
			return ret;
		}
		if ((unsigned int)len_user < strlen(mark_s)+pe-ps-strlen(mark_user_s)+strlen(mark_e))
		{
			return ret;
		}
		p = out_user;
		memcpy(p, mark_s, strlen(mark_s));
		p += strlen(mark_s);
		memcpy(p, ps+strlen(mark_user_s), pe-ps-strlen(mark_user_s));
		p += pe-ps-strlen(mark_user_s);
		memcpy(p, mark_e, strlen(mark_e));
		p += strlen(mark_e);

		ps = findstrmark_mem(buf, len, mark_ec_s);
		pe = findstrmark_mem(buf, len, mark_ec_e);
		if (NULL==ps || NULL==pe || ps>=pe)
		{
			return ret;
		}
		if (len_user-strlen((char*)out_user) < pe-ps+strlen(mark_ec_e))
		{
			return ret;
		}
		memcpy(p, ps, pe-ps+strlen(mark_ec_e));
	}
	ret = 0;
	return ret;
}
