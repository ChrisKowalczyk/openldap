/* $OpenLDAP$ */
/* bind.c - tcl bind routines
 *
 * Copyright 1999, Ben Collins <bcollins@debian.org>, All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted only
 * as authorized by the OpenLDAP Public License.  A copy of this
 * license is available at http://www.OpenLDAP.org/license.html or
 * in file LICENSE in the top-level directory of the distribution.
 */

#include "portable.h"

#include <stdio.h>

#include "slap.h"
#include "tcl_back.h"

int
tcl_back_bind (
	Operation * op,
	SlapReply * rs )
{
	char *command, *results;
	struct berval suf_tcl;
	int code;
	struct tclinfo *ti = (struct tclinfo *) op->o_bd->be_private;

	if (ti->ti_bind.bv_len == 0) {
		send_ldap_error (op, rs, LDAP_UNWILLING_TO_PERFORM,
			"bind not implemented" );
		return (-1);
	}

	if (tcl_merge_bvlist (op->o_bd->be_suffix, &suf_tcl) == NULL) {
		send_ldap_error (op, rs, LDAP_OTHER, NULL );
		return (-1);
	}

	command = (char *) ch_malloc (ti->ti_bind.bv_len + suf_tcl.bv_len +
		op->o_req_dn.bv_len + op->oq_bind.rb_cred.bv_len + 84);
	sprintf (command, "%s BIND {%ld/%ld} {%s} {%s} {%d} {%lu} {%s}",
		ti->ti_bind.bv_val, op->o_connid, (long) op->o_msgid,
		suf_tcl.bv_val, 
		op->o_req_dn.bv_val, op->oq_bind.rb_method, op->oq_bind.rb_cred.bv_len, op->oq_bind.rb_cred.bv_val);
	Tcl_Free (suf_tcl.bv_val);

	ldap_pvt_thread_mutex_lock (&tcl_interpreter_mutex);
	code = Tcl_GlobalEval (ti->ti_ii->interp, command);
	results = (char *) ch_strdup (ti->ti_ii->interp->result);
	ldap_pvt_thread_mutex_unlock (&tcl_interpreter_mutex);
	free (command);

	if (code != TCL_OK) {
		rs->sr_err = LDAP_OTHER;
		Debug (LDAP_DEBUG_SHELL, "tcl_bind_error: %s\n", results, 0, 0);
	} else {
		rs->sr_err = interp_send_results (op, rs, results);
	}

	if (rs->sr_err != LDAP_SUCCESS) {
		rs->sr_text = "internal backend error";
		send_ldap_result (op, rs);
	}

	free (results);
	return (rs->sr_err);
}
