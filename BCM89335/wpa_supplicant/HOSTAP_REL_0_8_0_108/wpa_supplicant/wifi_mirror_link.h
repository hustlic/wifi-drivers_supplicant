#ifndef WIFI_MIRROR_LINK_H
#define WIFI_MIRROR_LINK_H


int wifi_mirror_link_init(struct wpa_global *global);
void  wifi_mirror_link_deinit(struct wpa_global *global);
struct wpabuf* wifi_mrlnk_encaps(struct wpabuf *subelems);
int wifi_mirror_link_update_subelem(struct wpa_global *global);
int wifi_mirror_link_ie_set(struct wpa_global *global, char *cmd);
char * wifi_mirror_link_subelem_hex(const struct wpabuf *mrlnk_subelems);
extern int p2p_set_mirror_link_ie(struct p2p_data *p2p, struct wpabuf *buf);
extern int p2p_set_mirror_link_upnp_subelem(struct p2p_data *p2p, struct wpabuf *buf);
extern int p2p_set_mirror_link_internet_access_subelem(struct p2p_data *p2p, struct wpabuf *buf);

#endif
