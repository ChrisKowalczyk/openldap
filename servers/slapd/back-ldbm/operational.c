/* operational.c - ldbm backend operational attributes function */
/*
 * Copyright 1998-2003 The OpenLDAP Foundation, All Rights Reserved.
 * COPYING RESTRICTIONS APPLY, see COPYRIGHT file
 */

#include "portable.h"

#include <stdio.h>

#include <ac/string.h>
#include <ac/socket.h>

#include "slap.h"
#include "back-ldbm.h"
#include "proto-back-ldbm.h"

/*
 * sets *hasSubordinates to LDAP_COMPARE_TRUE/LDAP_COMPARE_FALSE
 * if the entry has children or not.
 */
int
ldbm_back_hasSubordinates(
	Operation	*op,
	Entry		*e,
	int		*hasSubordinates )
{
	if ( has_children( op->o_bd, e ) ) {
		*hasSubordinates = LDAP_COMPARE_TRUE;

	} else {
		*hasSubordinates = LDAP_COMPARE_FALSE;
	}

	return 0;
}

/*
 * sets the supported operational attributes (if required)
 */
int
ldbm_back_operational(
	Operation	*op,
	SlapReply	*rs,
	int		opattrs,
	Attribute	**a )
{
	Attribute	**aa = a;

	assert( rs->sr_entry );

	if ( opattrs || ad_inlist( slap_schema.si_ad_hasSubordinates, rs->sr_attrs ) ) {
		int	hs;

		hs = has_children( op->o_bd, rs->sr_entry );
		*aa = slap_operational_hasSubordinate( hs );
		if ( *aa != NULL ) {
			aa = &(*aa)->a_next;
		}
	}
	
	return 0;
}

