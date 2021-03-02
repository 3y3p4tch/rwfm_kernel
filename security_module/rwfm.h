/*
 * rwfm.h - definitions for RWFM structures
 * 
 * Written by Saurav Yadav <sauravyad@protonmail.com> Mar 2021
 */

#include <linux/uidgid.h>
#include <linux/types.h>
#include <linux/kernel.h>

#define RWFM_MAX_USERS 512

/*
 * RWFM user set - A string of booleans, where 1 at position `p` represents
 * user `p` is in the set, 0 represents that user is not in the set
 */
typedef struct {
	u64 val[DIV_ROUND_UP(RWFM_MAX_USERS, 64)];
} rwfm_uset_t;

static inline bool user_set_eq(rwfm_uset_t *s1, rwfm_uset_t *s2)
{
	bool tmp = true;
	for (int i = 0; i < DIV_ROUND_UP(RWFM_MAX_USERS, 64); i++)
		tmp &= s1->val[i] == s2->val[i];
	return tmp;
}

static inline rwfm_uset_t user_set_union(rwfm_uset_t *s1, rwfm_uset_t *s2)
{
	rwfm_uset_t tmp;
	for (int i = 0; i < DIV_ROUND_UP(RWFM_MAX_USERS, 64); i++)
		tmp.val[i] = s1->val[i] | s2->val[i];
	return tmp;
}

static inline rwfm_uset_t user_set_intersection(rwfm_uset_t *s1,
						rwfm_uset_t *s2)
{
	rwfm_uset_t tmp;
	for (int i = 0; i < DIV_ROUND_UP(RWFM_MAX_USERS, 64); i++)
		tmp.val[i] = s1->val[i] & s2->val[i];
	return tmp;
}

static inline bool is_subset_of(rwfm_uset_t *s1, rwfm_uset_t *s2)
{
	bool tmp;
	for (int i = 0; i < DIV_ROUND_UP(RWFM_MAX_USERS, 64); i++)
		tmp &= s1->val[i] == (s1->val[i] & s2->val[i]);
	return tmp;
}

static inline bool is_superset_of(rwfm_uset_t *s1, rwfm_uset_t *s2)
{
	return is_subset_of(s2, s1);
}

static inline bool is_user_in_set(kuid_t user, rwfm_uset_t *s)
{
	return s->val[__kuid_val(user) / 64] & (__kuid_val(user) % 64);
}

static inline void add_user_to_set(kuid_t user, rwfm_uset_t *s)
{
	s->val[__kuid_val(user) / 64] |= 1 << (__kuid_val(user) % 64);
}

static inline void remove_user_from_set(kuid_t user, rwfm_uset_t *s)
{
	s->val[__kuid_val(user) / 64] &= ~(1 << (__kuid_val(user) % 64));
}

struct subject {
	pid_t pid;
	kuid_t owner;
	rwfm_uset_t readers;
	rwfm_uset_t writers;
};

struct object {
	dev_t device_id;
	ino_t inode_number;
	kuid_t owner;
	rwfm_uset_t readers;
	rwfm_uset_t writers;
};
