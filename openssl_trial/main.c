/*
 * main.c
 *
 *  Created on: Nov 17, 2017
 *      Author: root
 */


#include <stdio.h>
#include <stdlib.h>

#include <libconfig.h>
#include <string.h>
#include "main.h"
#include "ieee802_11.h"

#define COUNTER_INFINITY        65535

#define state_to_string(x) (x) == SAE_NOTHING ? "NOTHING" : \
                           (x) == SAE_COMMITTED ? "COMMITTED" : \
                           (x) == SAE_CONFIRMED ? "CONFIRMED" : \
                           (x) == SAE_ACCEPTED ? "ACCEPTED" : \
                           "unknown"
#define seq_to_string(x) (x) == SAE_AUTH_COMMIT ? "COMMIT" : \
                         (x) == SAE_AUTH_CONFIRM ? "CONFIRM" : \
                         "unknown"

#define status_to_string(x) (x) == WLAN_STATUS_ANTI_CLOGGING_TOKEN_NEEDED ? "TOKEN NEEDED" : \
                            (x) == WLAN_STATUS_NOT_SUPPORTED_GROUP ? "REJECTION" : \
                            "unknown"


unsigned int function_mdlen = SHA256_DIGEST_LENGTH;

/*
 * the functions H() and CN()
 */
#define H_Init(ctx,x,l) HMAC_Init((ctx), (x), (l), EVP_sha256())
#define H_Update(ctx,x,l) HMAC_Update((ctx),(x),(l))
#define H_Final(ctx,x) HMAC_Final((ctx), (x), &function_mdlen)

#define CN_Init(ctx,x,l) HMAC_Init((ctx), (x), (l), EVP_sha256())
#define CN_Update(ctx,x,l) HMAC_Update((ctx),(x),(l))
#define CN_Final(ctx,x) HMAC_Final((ctx), (x), &function_mdlen)

/*
 * global variables
 */
BN_CTX *bnctx = NULL;
GD *gd;                                 /* group definitions */
BIO *out;
char allzero[SHA256_DIGEST_LENGTH];
unsigned long token_generator;
int curr_open, open_threshold, retrans;
unsigned long blacklist_timeout, giveup_threshold, pmk_expiry;
unsigned int sae_debug_mask;

static int next_candidate_id = 0;



#if 0
int main(void) {
	BIGNUM *x, *y, *mask;

    if (((mask = BN_new()) == NULL)) {
        puts("unable to commit to peer!\n");
        return -1;
    }
    else{
		puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
		return EXIT_SUCCESS;
    }
}
#else
int main(void) {
	char *conffile = "/work/C_Project/openssl_trial/authsae.sample.cfg";
	char *meshid="";
    struct sae_config sae_conf;

    struct candidate *peer;
    unsigned char herMac[ETH_ALEN]={0x10,0x20,0x30,0x00,0x00,0x00};
    unsigned char myMac[ETH_ALEN]={0x40,0x50,0x60,0x00,0x00,0x00};


    //sleep(20);

    if (conffile) {
        struct config_t cfg;
        struct config_setting_t *section;

        config_init(&cfg);
        if (!config_read_file(&cfg, conffile)) {
            fprintf(stderr, "Failed to load config file %s: %s\n", conffile,
                    config_error_text(&cfg));
            return -1;
        }
        section = config_lookup(&cfg, "authsae.sae");
        if (section == NULL) {
            fprintf(stderr, "Config file has not sae section\n");
            return -1;
        }

        if (sae_parse_libconfig(section, &sae_conf) != 0) {
            fprintf(stderr, "Failed to parse SAE configuration.\n");
            return -1;
        }

        config_destroy(&cfg);
    }

    if (sae_initialize(meshid, &sae_conf) < 0) {
        fprintf(stderr, "cannot configure SAE, check config file!\n");
        exit(EXIT_FAILURE);
    }

#if 1
    if ((peer = create_candidate(&herMac[0], &myMac[0], 0, (void*)0/*cookie*/)) == NULL) {
        return -1;
    }
    peer->cookie = (void*)NULL/*cookie*/;
    /*
     * assign the first group in the list as the one to try
     */
    if (assign_group_to_peer(peer, gd) < 0) {
        //fin(WLAN_STATUS_UNSPECIFIED_FAILURE, peer->peer_mac, NULL, 0, peer->cookie);
        //delete_peer(&peer);
    	return -1;
    } else {
#if 1
        commit_to_peer(peer, NULL, 0);
        //peer->t0 = srv_add_timeout(srvctx, SRV_SEC(retrans), retransmit_peer, peer);
        peer->state = SAE_COMMITTED;
        fprintf(stderr, "state of " MACSTR " is now (%d) %s\n\n",
                  MAC2STR(peer->peer_mac), peer->state, state_to_string(peer->state));
#endif
    }

#endif
	return EXIT_SUCCESS;
}

static int
compute_group_definition (GD *grp, char *password, unsigned short num)
{
    BIGNUM *cofactor = NULL;
    int nid, ret = 0;

    switch (num) {        /* from IANA registry for IKE D-H groups */
        case 19:
            nid = NID_X9_62_prime256v1;
            break;
        case 20:
            nid = NID_secp384r1;
            break;
        case 21:
            nid = NID_secp521r1;
            break;
        case 25:
            nid = NID_X9_62_prime192v1;
            break;
        case 26:
            nid = NID_secp224r1;
            break;
        default:
            printf("unsupported group %d\n", num);
            return -1;
    }
    if ((grp->group = EC_GROUP_new_by_curve_name(nid)) == NULL) {
        puts("unable to create EC_GROUP!\n");
        return -1;
    }
    cofactor = NULL; grp->order = NULL; grp->prime = NULL;

    if (((cofactor = BN_new()) == NULL) ||
        ((grp->order = BN_new()) == NULL) ||
        ((grp->prime = BN_new()) == NULL)) {
        puts("unable to create bignums!\n");
        goto fail;
    }
    if (!EC_GROUP_get_curve_GFp(grp->group, grp->prime, NULL, NULL, bnctx)) {
        puts("unable to get prime for GFp curve!\n");
        goto fail;
    }
    if (!EC_GROUP_get_order(grp->group, grp->order, bnctx)) {
    	puts("unable to get order for curve!\n");
        goto fail;
    }
    if (!EC_GROUP_get_cofactor(grp->group, cofactor, bnctx)) {
    	puts("unable to get cofactor for curve!\n");
        goto fail;
    }
    if (BN_cmp(cofactor, BN_value_one())) {
        /*
         * no curves with a co-factor > 1 are allowed. Technically they will work
         * but the special handling of them to deal with a small sub-group attack
         * when compared to how many of them are in this IANA registry (as of writing
         * this that number is zero) makes it better to just not support them.
         */
    	puts("attempting to use a curve with a co-factor > 1!\n");
        goto fail;
    }
    grp->group_num = num;
    strncpy(grp->password, password, sizeof(grp->password));

    if (0) {
fail:
        BN_free(grp->order);
        BN_free(grp->prime);
        ret = -1;
    }
    BN_free(cofactor);

    return ret;
}

int
sae_initialize (char *ourssid, struct sae_config *config)
{
    GD *curr, *prev = NULL;
    int i;

    /* TODO: detect duplicate calls.  validate config. */

    EVP_add_digest(EVP_sha256());
    if ((out = BIO_new(BIO_s_file())) == NULL) {
        fprintf(stderr, "SAE: unable to create file BIO!\n");
        return -1;
    }
    BIO_set_fp(out, stderr, BIO_NOCLOSE);

    /*
     * initialize globals
     */
    memset(allzero, 0, SHA256_DIGEST_LENGTH);
#if 0
JC: Commented out until we decide whether this is needed (in which case we must
    be prepared to accept a binary, non-null terminated mesh ID) or not (the
    mesh_ssid is not used anywhere in this module, so maybe it can be dumped).

    memcpy(mesh_ssid, ourssid, strlen(ourssid));
#endif
    TAILQ_INIT(&peers);
    TAILQ_INIT(&blacklist);
    RAND_pseudo_bytes((unsigned char *)&token_generator, sizeof(unsigned long));
    if ((bnctx = BN_CTX_new()) == NULL) {
        fprintf(stderr, "cannot create bignum context!\n");
        return -1;
    }
    /*
     * set defaults and read in config
     */
    sae_debug_mask = config->debug;
    curr_open = 0;
    open_threshold = config->open_threshold ? config->open_threshold : 5;
    blacklist_timeout = config->blacklist_timeout ? config->blacklist_timeout
        : 30;
    giveup_threshold = config->giveup_threshold ? config->giveup_threshold :
        5;
    retrans = config->retrans ? config->retrans : 3;
    pmk_expiry = config->pmk_expiry ? config->pmk_expiry : 86400; /* one day */
    /*
     * create groups from configuration data
     */
    for (i=0; i<config->num_groups; i++) {
        if ((curr = (GD *)malloc(sizeof(GD))) == NULL) {
        	puts("cannot malloc group definition!\n");
            return -1;
        }
        if (compute_group_definition(curr, config->pwd, config->group[i])) {
            free(curr);
            continue;
        }
        if (prev)
            prev->next = curr;
        else
            gd = curr;
        prev = curr;
        curr->next = NULL;
#if 1
        printf("group %d is configured, prime is %d"
                " bytes\n", curr->group_num, BN_num_bytes(curr->prime));
#endif
    }
    return 1;
}

int
sae_parse_libconfig (struct config_setting_t *sae_section, struct sae_config* config)
{
    struct config_setting_t *setting, *group;
    char *pwd;

    memset(config, 0, sizeof(struct sae_config));
    config_setting_lookup_int(sae_section, "debug", (config_int_t *)&config->debug);
    setting = config_setting_get_member(sae_section, "group");
    if (setting != NULL) {
        while (1) {
            group = config_setting_get_elem(setting, config->num_groups);
            if (!group)
                break;
            config->group[config->num_groups] =
                config_setting_get_int_elem(setting, config->num_groups);
            config->num_groups++;
            if (config->num_groups == SAE_MAX_EC_GROUPS)
                break;
        }
    }
    if (config_setting_lookup_string(sae_section, "password", (const char **)&pwd)) {
        strncpy(config->pwd, pwd, SAE_MAX_PASSWORD_LEN);
        if (config->pwd[SAE_MAX_PASSWORD_LEN - 1] != 0) {
            fprintf(stderr, "WARNING: Truncating password\n");
            config->pwd[SAE_MAX_PASSWORD_LEN - 1] = 0;
        }
    }
    config_setting_lookup_int(sae_section, "retrans", (config_int_t *)&config->retrans);
    config_setting_lookup_int(sae_section, "lifetime", (config_int_t *)&config->pmk_expiry);
    config_setting_lookup_int(sae_section, "thresh", (config_int_t *)&config->open_threshold);
    config_setting_lookup_int(sae_section, "blacklist", (config_int_t *)&config->blacklist_timeout);
    config_setting_lookup_int(sae_section, "giveup", (config_int_t *)&config->giveup_threshold);
    return 0;
}

/*
 * calculate the legendre symbol (a/p)
 */
static int
legendre (BIGNUM *a, BIGNUM *p, BIGNUM *exp, BN_CTX *bnctx)
{
    BIGNUM *tmp = NULL;
    int symbol = -1;

    if ((tmp = BN_new()) != NULL) {
        BN_mod_exp(tmp, a, exp, p, bnctx);
        if (BN_is_word(tmp, 1))
                symbol = 1;
        else if (BN_is_zero(tmp))
                symbol = 0;
        else
                symbol = -1;

        BN_free(tmp);
    }
    return symbol;
}
int
prf (unsigned char *key, int keylen, unsigned char *label, int labellen,
     unsigned char *context, int contextlen,
     unsigned char *result, int resultbitlen)
{
    HMAC_CTX ctx;
    unsigned char digest[SHA256_DIGEST_LENGTH];
    int resultlen, len = 0;
    unsigned int mdlen = SHA256_DIGEST_LENGTH;
    unsigned char mask = 0xff;
    unsigned short reslength;
    unsigned short i = 0, i_le;

    reslength = ieee_order(resultbitlen);
    resultlen = (resultbitlen + 7)/8;
    do {
        i++;
        HMAC_Init(&ctx, key, keylen, EVP_sha256());
        i_le = ieee_order(i);
        HMAC_Update(&ctx, (unsigned char *) &i_le, sizeof(i_le));
        HMAC_Update(&ctx, label, labellen);
        HMAC_Update(&ctx, context, contextlen);
        HMAC_Update(&ctx, (unsigned char *)&reslength, sizeof(unsigned short));
        HMAC_Final(&ctx, digest, &mdlen);
        if ((len + mdlen) > resultlen) {
            memcpy(result+len, digest, resultlen - len);
        } else {
            memcpy(result+len, digest, mdlen);
        }
        len += mdlen;
        HMAC_CTX_cleanup(&ctx);
    } while (len < resultlen);
    /*
     * we're expanding to a bit length, if this is not a
     * multiple of 8 bits then mask off the excess.
     */
    if (resultbitlen % 8) {
        mask <<= (8 - (resultbitlen % 8));
        result[resultlen - 1] &= mask;
    }
    return resultlen;
}

struct candidate *
create_candidate(unsigned char *her_mac, unsigned char *my_mac, unsigned short got_token, void *cookie)
{
    struct candidate *peer;

    if ((peer = (struct candidate *)malloc(sizeof(struct candidate))) == NULL) {
    	fprintf(stderr,"can't malloc space for candidate!\n");
        return NULL;
    }
    memset(peer, 0, sizeof(*peer));
    memcpy(peer->my_mac, my_mac, ETH_ALEN);
    memcpy(peer->peer_mac, her_mac, ETH_ALEN);
    if (((peer->peer_scalar = BN_new()) == NULL) ||
        ((peer->my_scalar = BN_new()) == NULL)) {
    	fprintf(stderr, "can't create peer data structures!\n");
        free(peer);
        return NULL;
    }
    peer->got_token = got_token;
    peer->failed_auth = peer->beacons = peer->state = peer->sync = peer->sc = peer->rc = 0;
    peer->private_val = NULL;
    peer->pwe = peer->peer_element = peer->my_element = NULL;
    TAILQ_INSERT_TAIL(&peers, peer, entry);
    peer->state = SAE_NOTHING;
    peer->cookie = cookie;
    peer->candidate_id = next_candidate_id++;
    curr_open++;

    //peer_created(her_mac);

    return peer;
}


/*
 * assign_group_tp_peer()
 *      The group has been selected, assign it to the peer and create PWE.
 */
int
assign_group_to_peer (struct candidate *peer, GD *grp)
{
    HMAC_CTX ctx;
    BIGNUM *x_candidate = NULL, *x = NULL, *rnd = NULL, *qr = NULL, *qnr = NULL;
    BIGNUM *pm1 = NULL, *pm1d2 = NULL, *tmp1 = NULL, *tmp2 = NULL, *a = NULL, *b = NULL;
    unsigned char pwe_digest[SHA256_DIGEST_LENGTH], addrs[ETH_ALEN * 2], ctr;
    unsigned char *prfbuf = NULL, *primebuf = NULL;
    int primebitlen, is_odd, check, found = 0;

    /*
     * allow for replacement of group....
     */
    EC_POINT_free(peer->pwe);
    peer->pwe = NULL;
    EC_POINT_free(peer->peer_element);
    peer->peer_element = NULL;
    EC_POINT_free(peer->my_element);
    peer->my_element = NULL;
    BN_free(peer->private_val);
    peer->private_val = NULL;

    if (((rnd = BN_new()) == NULL) ||
        ((pm1d2 = BN_new()) == NULL) ||
        ((pm1 = BN_new()) == NULL) ||
        ((tmp1 = BN_new()) == NULL) ||
        ((tmp2 = BN_new()) == NULL) ||
        ((a = BN_new()) == NULL) ||
        ((b = BN_new()) == NULL) ||
        ((qr = BN_new()) == NULL) ||
        ((qnr = BN_new()) == NULL) ||
        ((x_candidate = BN_new()) == NULL)) {
    	fprintf(stderr,  "can't create bignum for candidate!\n");
        goto fail;
    }
    peer->grp_def = grp;
    peer->pwe = EC_POINT_new(grp->group);
    peer->peer_element = EC_POINT_new(grp->group);
    peer->my_element = EC_POINT_new(grp->group);

    if ((prfbuf = (unsigned char *)malloc(BN_num_bytes(grp->prime))) == NULL) {
    	fprintf(stderr,  "unable to malloc space for prf buffer!\n");
        BN_free(rnd);
        BN_free(x_candidate);
	goto fail;
    }
    if ((primebuf = (unsigned char *)malloc(BN_num_bytes(grp->prime))) == NULL) {
    	fprintf(stderr, "unable to malloc space for prime!\n");
        free(prfbuf);
        BN_free(rnd);
        BN_free(x_candidate);
	goto fail;
    }
    BN_bn2bin(grp->prime, primebuf);
    primebitlen = BN_num_bits(grp->prime);

    if (!EC_GROUP_get_curve_GFp(grp->group, NULL, a, b, NULL)) {
        free(prfbuf);
    }

    BN_sub(pm1, grp->prime, BN_value_one());
    BN_copy(tmp1, BN_value_one());
    BN_add(tmp1, tmp1, BN_value_one());
    BN_div(pm1d2, tmp2, pm1, tmp1, bnctx);      /* (p-1)/2 */

    /*
     * generate a random quadratic residue modulo p and a random
     * quadratic non-residue modulo p.
     */
    do {
        BN_rand_range(qr, pm1);
    } while (legendre(qr, grp->prime, pm1d2, bnctx) != 1);
    do {
        BN_rand_range(qnr, pm1);
    } while (legendre(qnr, grp->prime, pm1d2, bnctx) != -1);
    memset(prfbuf, 0, BN_num_bytes(grp->prime));

    fprintf(stderr,  "computing PWE on %d bit curve number %d\n", primebitlen, grp->group_num);
    ctr = 0;
    while (ctr < 40) {
        ctr++;
        /*
         * compute counter-mode password value and stretch to prime
         */
        if (memcmp(peer->peer_mac, peer->my_mac, ETH_ALEN) > 0) {
            memcpy(addrs, peer->peer_mac, ETH_ALEN);
            memcpy(addrs+ETH_ALEN, peer->my_mac, ETH_ALEN);
        } else {
            memcpy(addrs, peer->my_mac, ETH_ALEN);
            memcpy(addrs+ETH_ALEN, peer->peer_mac, ETH_ALEN);
        }
        H_Init(&ctx, addrs, (ETH_ALEN * 2));
        H_Update(&ctx, (unsigned char *) grp->password, strlen(grp->password));
        H_Update(&ctx, &ctr, sizeof(ctr));
        H_Final(&ctx, pwe_digest);
        HMAC_CTX_cleanup(&ctx);

#if 0
        if (sae_debug_mask & SAE_DEBUG_CRYPTO_VERB) {
            if (memcmp(peer->peer_mac, peer->my_mac, ETH_ALEN) > 0) {
                printf("H(" MACSTR " | " MACSTR ", %s | %d)\n",
                       MAC2STR(peer->peer_mac), MAC2STR(peer->my_mac), grp->password, ctr);
            } else {
                printf("H(" MACSTR " | " MACSTR ", %s | %d)\n",
                       MAC2STR(peer->my_mac), MAC2STR(peer->peer_mac), grp->password, ctr);
            }
            dump_buffer(pwe_digest, SHA256_DIGEST_LENGTH);
        }
#endif

        BN_bin2bn(pwe_digest, SHA256_DIGEST_LENGTH, rnd);
        prf(pwe_digest, SHA256_DIGEST_LENGTH,
            (unsigned char *)"SAE Hunting and Pecking", strlen("SAE Hunting and Pecking"),
            primebuf, BN_num_bytes(grp->prime),
            prfbuf, primebitlen);
        BN_bin2bn(prfbuf, BN_num_bytes(grp->prime), x_candidate);
        /*
         * prf() returns a string of bits 0..primebitlen, but BN_bin2bn will
         * treat that string of bits as a big-endian number. If the primebitlen
         * is not an even multiple of 8 we masked off the excess bits-- those
         * _after_ primebitlen-- in prf() so now interpreting this as a
         * big-endian number is wrong. We have to shift right the amount we
         * masked off.
         */
        if (primebitlen % 8) {
            BN_rshift(x_candidate, x_candidate, (8 - (primebitlen % 8)));
        }

        /*
         * if this candidate value is greater than the prime then try again
         */
        if (BN_ucmp(x_candidate, grp->prime) >= 0) {
            continue;
        }

#if 0
        if (sae_debug_mask & SAE_DEBUG_CRYPTO_VERB) {
            memset(prfbuf, 0, BN_num_bytes(grp->prime));
            BN_bn2bin(x_candidate, prfbuf + (BN_num_bytes(grp->prime) - BN_num_bytes(x_candidate)));
            print_buffer("candidate x value", prfbuf, BN_num_bytes(grp->prime));
        }
#endif

        /*
         * compute y^2 using the equation of the curve
         *
         *      y^2 = x^3 + ax + b
         */
        BN_mod_sqr(tmp1, x_candidate, grp->prime, bnctx);
        BN_mod_mul(tmp2, tmp1, x_candidate, grp->prime, bnctx);
        BN_mod_mul(tmp1, a, x_candidate, grp->prime, bnctx);
        BN_mod_add_quick(tmp2, tmp2, tmp1, grp->prime);
        BN_mod_add_quick(tmp2, tmp2, b, grp->prime);

        /*
         * mask tmp2 so doing legendre won't leak timing info
         *
         * tmp1 is a random number between 1 and p-1
         */
        BN_rand_range(tmp1, pm1);

        BN_mod_mul(tmp2, tmp2, tmp1, grp->prime, bnctx);
        BN_mod_mul(tmp2, tmp2, tmp1, grp->prime, bnctx);
        /*
         * now tmp2 (y^2) is masked, all values between 1 and p-1
         * are equally probable. Multiplying by r^2 does not change
         * whether or not tmp2 is a quadratic residue, just masks it.
         *
         * flip a coin, multiply by the random quadratic residue or the
         * random quadratic nonresidue and record heads or tails
         */
        if (BN_is_odd(tmp1)) {
            BN_mod_mul(tmp2, tmp2, qr, grp->prime, bnctx);
            check = 1;
        } else {
            BN_mod_mul(tmp2, tmp2, qnr, grp->prime, bnctx);
            check = -1;
        }

        /*
         * now it's safe to do legendre, if check is 1 then it's
         * a straightforward test (multiplying by qr does not
         * change result), if check is -1 then its the opposite test
         * (multiplying a qr by qnr would make a qnr)
         */
        if (legendre(tmp2, grp->prime, pm1d2, bnctx) == check) {
            if (found == 1) {
                continue;
            }
            /*
             * need to unambiguously identify the solution, if there is one...
             */
            if (BN_is_odd(rnd)) {
                is_odd = 1;
            } else {
                is_odd = 0;
            }
            if ((x = BN_dup(x_candidate)) == NULL) {
                goto fail;
            }
            fprintf(stderr,  "it took %d tries to find PWE: %d\n", ctr, grp->group_num);
            found = 1;
        }
    }
    /*
     * 2^-40 is about one in a trillion so we should always find a point.
     * When we do, we know x^3 + ax + b is a quadratic residue so we can
     * assign a point using x and our discriminator (is_odd)
     */
    if ((found == 0) ||
        (!EC_POINT_set_compressed_coordinates_GFp(grp->group, peer->pwe, x, is_odd, bnctx))) {
        EC_POINT_free(peer->pwe);
        peer->pwe = NULL;
    }

#if 0
    if (found && peer->pwe && (sae_debug_mask & SAE_DEBUG_CRYPTO_VERB)) {
        BIGNUM *px = NULL, *py = NULL;
        if (((px = BN_new()) != NULL) &&
            ((py = BN_new()) != NULL)) {
            if (EC_POINT_get_affine_coordinates_GFp(peer->grp_def->group, peer->pwe, px, py, bnctx)) {
                printf("PWE (x,y):\n");
                memset(prfbuf, 0, BN_num_bytes(grp->prime));
                BN_bn2bin(px, prfbuf + (BN_num_bytes(grp->prime) - BN_num_bytes(px)));
                print_buffer("x", prfbuf, BN_num_bytes(grp->prime));
                memset(prfbuf, 0, BN_num_bytes(grp->prime));
                BN_bn2bin(py, prfbuf + (BN_num_bytes(grp->prime) - BN_num_bytes(py)));
                print_buffer("y", prfbuf, BN_num_bytes(grp->prime));
            }
            BN_free(px);
            BN_free(py);
        }
    }
#endif

fail:
    if (prfbuf != NULL) {
        free(prfbuf);
    }
    if (primebuf != NULL) {
        free(primebuf);
    }
    if (found) {
        BN_free(x);
    }

    BN_free(x_candidate);
    BN_free(rnd);
    BN_free(pm1d2);
    BN_free(pm1);
    BN_free(tmp1);
    BN_free(tmp2);
    BN_free(a);
    BN_free(b);
    BN_free(qr);
    BN_free(qnr);

    if (peer->pwe == NULL) {
    	fprintf(stderr, "unable to find random point on curve for group %d, something's fishy!\n",
                  grp->group_num);
        return -1;
    }
    fprintf(stderr, "assigning group %d to peer, the size of the prime is %d\n",
              peer->grp_def->group_num, BN_num_bytes(peer->grp_def->prime));

    return 0;
}

int
commit_to_peer (struct candidate *peer, unsigned char *token, int token_len)
{
    char buf[2048];
    struct ieee80211_mgmt_frame *frame;
    int offset1, offset2;
    size_t len = 0;
    BIGNUM *x, *y, *mask;
    unsigned short grp_num;
    unsigned char *ptr;

    memset(buf, 0, sizeof(buf));
    frame = (struct ieee80211_mgmt_frame *)buf;

    /*
     * fill in authentication frame header...
     */
    frame->frame_control = ieee_order((IEEE802_11_FC_TYPE_MGMT << 2 | IEEE802_11_FC_STYPE_AUTH << 4));
    memcpy(frame->sa, peer->my_mac, ETH_ALEN);
    memcpy(frame->da, peer->peer_mac, ETH_ALEN);
    memcpy(frame->bssid, peer->peer_mac, ETH_ALEN);

    frame->authenticate.alg = ieee_order(SAE_AUTH_ALG);
    frame->authenticate.auth_seq = ieee_order(SAE_AUTH_COMMIT);
    len = IEEE802_11_HDR_LEN + sizeof(frame->authenticate);
    ptr = frame->authenticate.u.var8;

    /*
     * first, indicate what group we're committing with
     */
    grp_num = ieee_order(peer->grp_def->group_num);
    memcpy(ptr, &grp_num, sizeof(unsigned short));
    ptr += sizeof(unsigned short);
    len += sizeof(unsigned short);

    /*
     * if we've been asked to include a token then include a token
     */
    if (token_len && (token != NULL)) {
        memcpy(ptr, token, token_len);
        ptr += token_len;
        len += token_len;
    }

    if (peer->private_val == NULL) {
        if (((mask = BN_new()) == NULL) ||
            ((peer->private_val = BN_new()) == NULL)) {
        	fprintf(stderr, "unable to commit to peer!\n");
            return -1;
        }
        /*
         * generate private values
         */
        BN_rand_range(peer->private_val, peer->grp_def->order);
        BN_rand_range(mask, peer->grp_def->order);
#if 0
        if (sae_debug_mask & SAE_DEBUG_CRYPTO_VERB) {
            pp_a_bignum("local private value", peer->private_val);
            pp_a_bignum("local mask value", mask);
        }
#endif
        /*
         * generate scalar = (priv + mask) mod order
         */
        BN_add(peer->my_scalar, peer->private_val, mask);
        BN_mod(peer->my_scalar, peer->my_scalar, peer->grp_def->order, bnctx);
        /*
         * generate element = -(mask*pwe)
         */
        if (!EC_POINT_mul(peer->grp_def->group, peer->my_element, NULL, peer->pwe, mask, bnctx)) {
        	fprintf(stderr, "unable to compute A!\n");
            BN_free(mask);
            return -1;
        }
        if (!EC_POINT_invert(peer->grp_def->group, peer->my_element, bnctx)) {
        	fprintf(stderr, "unable to invert A!\n");
            BN_free(mask);
            return -1;
        }
        BN_free(mask);
    }
    if (((x = BN_new()) == NULL) ||
        ((y = BN_new()) == NULL)) {
    	fprintf(stderr, "unable to create x,y bignums\n");
        return -1;
    }
    if (!EC_POINT_get_affine_coordinates_GFp(peer->grp_def->group, peer->my_element, x, y, bnctx)) {
    	fprintf(stderr, "unable to get secret key!\n");
        BN_free(x);
        BN_free(y);
        return -1;
    }
#if 0
    if (sae_debug_mask & SAE_DEBUG_CRYPTO_VERB) {
        printf("local commit:\n");
        pp_a_bignum("my scalar", peer->my_scalar);
        printf("my element:\n");
        pp_a_bignum("x", x);
        pp_a_bignum("y", y);
    }
#endif
    /*
     * fill in the commit, first in the commit message is the scalar
     */
    offset1 = BN_num_bytes(peer->grp_def->order) - BN_num_bytes(peer->my_scalar);
    BN_bn2bin(peer->my_scalar, ptr + offset1);
    ptr += BN_num_bytes(peer->grp_def->order);
    len += BN_num_bytes(peer->grp_def->order);

    /*
     * ...next is the element, x then y
     */
    if (!EC_POINT_get_affine_coordinates_GFp(peer->grp_def->group, peer->my_element, x, y, bnctx)) {
    	fprintf(stderr, "unable to determine u!\n");
        exit(1);
    }
    offset1 = BN_num_bytes(peer->grp_def->prime) - BN_num_bytes(x);
    BN_bn2bin(x, ptr + offset1);
    ptr += BN_num_bytes(peer->grp_def->prime);

    offset2 = BN_num_bytes(peer->grp_def->prime) - BN_num_bytes(y);
    BN_bn2bin(y, ptr + offset2);
    ptr += BN_num_bytes(peer->grp_def->prime);

    len += (2 * BN_num_bytes(peer->grp_def->prime));

    fprintf(stderr, "peer " MACSTR " in %s, sending %s (%s token), len %d, group %d\n",
              MAC2STR(peer->peer_mac),
              state_to_string(peer->state),
              seq_to_string(ieee_order(frame->authenticate.auth_seq)),
              (token_len ? "with" : "no"), len,
              peer->grp_def->group_num);
    BN_free(x);
    BN_free(y);

    //++ CowTwo: dump frame content
    fprintf(stderr, "frame=[");
    {
    	int ii;
    	unsigned char* dumpPtr= (unsigned char*)frame;
		for (ii=0;ii<len;ii++){
			fprintf(stderr, "0x%02x", (unsigned char)(*dumpPtr));
			if (ii<(len-1))
				fprintf(stderr, ", ");
			else
				fprintf(stderr, "];");
			if (ii%8==7)
				fprintf(stderr,"\n       ");
			dumpPtr+=1;
		}
    }
    //--

#if 0
    if (meshd_write_mgmt(buf, len, peer->cookie) != len) {
    	fprintf(stderr, "can't send an authentication frame to " MACSTR "\n",
                  MAC2STR(peer->peer_mac));
        return -1;
    }
#endif

    return 0;
}


#endif
