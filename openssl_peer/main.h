/*
 * sae.h
 *
 *  Created on: Nov 18, 2017
 *      Author: root
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/ossl_typ.h>
#include <openssl/sha.h>
#include <sys/queue.h>
#include "ieee802_11.h"


#define    SAE_MAX_EC_GROUPS    10
#define    SAE_MAX_PASSWORD_LEN 80

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */

struct sae_config {
    int group[SAE_MAX_EC_GROUPS];
    int num_groups;
    char pwd[SAE_MAX_PASSWORD_LEN];
    int debug;
    int retrans;
    int pmk_expiry;
    int open_threshold;
    int blacklist_timeout;
    int giveup_threshold;
};

#ifndef LIBCONFIG_SETTING_INT_AS_LONG
typedef int config_int_t;
#else
typedef long int config_int_t;
#endif


typedef struct group_def_ {
    unsigned short group_num;
    EC_GROUP *group;
    BIGNUM *order;
    BIGNUM *prime;
    char password[80];
    struct group_def_ *next;
} GD;
struct candidate {
    TAILQ_ENTRY(candidate) entry;
    GD *grp_def;
    EC_POINT *pwe;
    unsigned char pmkid[16];
    unsigned char pmk[SHA256_DIGEST_LENGTH];
    unsigned char kck[SHA256_DIGEST_LENGTH];
    BIGNUM *private_val;
    BIGNUM *peer_scalar;
    BIGNUM *my_scalar;
    EC_POINT *peer_element;
    EC_POINT *my_element;
    unsigned long beacons;
    unsigned int failed_auth;
    //timerid t0;
    //timerid t1;
#define SAE_NOTHING             0
#define SAE_COMMITTED           1
#define SAE_CONFIRMED           2
#define SAE_ACCEPTED            3
    unsigned short state;
    unsigned short got_token;
    unsigned short sync;
    unsigned short sc;
    unsigned short rc;
    unsigned char peer_mac[ETH_ALEN];
    unsigned char my_mac[ETH_ALEN];
    /*  AMPE related fields */
    //timerid t2;
    //enum plink_state link_state;
    //le16 my_lid;
    //le16 peer_lid;
    unsigned char my_nonce[32];
    unsigned char peer_nonce[32];
    unsigned short reason;
    unsigned short retries;
    unsigned int timeout;
    unsigned char aek[SHA256_DIGEST_LENGTH];
    unsigned char mtk[16];
    unsigned char mgtk[16];
    unsigned int mgtk_expiration;
    unsigned char igtk[16];
    //u16 igtk_keyid;
    //unsigned char sup_rates[MAX_SUPP_RATES];
    unsigned short sup_rates_len;
    //siv_ctx sivctx;
    void *cookie;
    struct ampe_config *conf;
    unsigned int ch_type; /* nl80211_channel_type */
    int candidate_id;

    //timerid rekey_ping_timer;
    unsigned int rekey_ping_count;
    unsigned int rekey_reauth_count;
    unsigned int rekey_ok;
    unsigned int rekey_ok_ping_rx;
};

struct candidate *find_peer(unsigned char *mac, int accept);
void delete_peer(struct candidate **peer);

TAILQ_HEAD(fubar, candidate) blacklist;
TAILQ_HEAD(blah, candidate) peers;

#define for_each_peer(peer) \
	TAILQ_FOREACH(peer, &peers, entry)


int
sae_parse_libconfig (struct config_setting_t *sae_section, struct sae_config* config);
int
sae_initialize (char *ourssid, struct sae_config *config);
struct candidate *
create_candidate(unsigned char *her_mac, unsigned char *my_mac, unsigned short got_token, void *cookie);
int
assign_group_to_peer (struct candidate *peer, GD *grp);
int
commit_to_peer (struct candidate *peer, unsigned char *token, int token_len);
int
process_commit (struct candidate *peer, struct ieee80211_mgmt_frame *frame, int len);
int
confirm_to_peer (struct candidate *peer);


#endif /* MAIN_H_ */
