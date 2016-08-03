/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "ProtocolModule"
 * 	found in "C:/devel/DbgToolkit/trace_proto/trace_proto.asn1"
 * 	`asn1c -gen-PER -fuse-int64 -S`
 */

#include "DictionaryMsg.h"

static int
memb_type_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 127)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_per_constraints_t asn_PER_memb_type_constr_2 GCC_NOTUSED = {
	{ asn_per_constraint_s::APC_CONSTRAINED,	 7,  7,  0,  127 }	/* (0..127) */,
	{ asn_per_constraint_s::APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_dict_3[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_DictPair,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		""
		},
};
static const ber_tlv_tag_t asn_DEF_dict_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_dict_specs_3 = {
	sizeof(struct DictionaryMsg::dict),
	offsetof(struct DictionaryMsg::dict, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_dict_3 = {
	"dict",
	"dict",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	SEQUENCE_OF_decode_uper,
	SEQUENCE_OF_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_dict_tags_3,
	sizeof(asn_DEF_dict_tags_3)
		/sizeof(asn_DEF_dict_tags_3[0]) - 1, /* 1 */
	asn_DEF_dict_tags_3,	/* Same as above */
	sizeof(asn_DEF_dict_tags_3)
		/sizeof(asn_DEF_dict_tags_3[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_dict_3,
	1,	/* Single element */
	&asn_SPC_dict_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_DictionaryMsg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DictionaryMsg, type),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		memb_type_constraint_1,
		&asn_PER_memb_type_constr_2,
		0,
		"type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DictionaryMsg, dict),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_dict_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"dict"
		},
};
static const ber_tlv_tag_t asn_DEF_DictionaryMsg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DictionaryMsg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* type */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* dict */
};
static asn_SEQUENCE_specifics_t asn_SPC_DictionaryMsg_specs_1 = {
	sizeof(struct DictionaryMsg),
	offsetof(struct DictionaryMsg, _asn_ctx),
	asn_MAP_DictionaryMsg_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DictionaryMsg = {
	"DictionaryMsg",
	"DictionaryMsg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	SEQUENCE_decode_uper,
	SEQUENCE_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DictionaryMsg_tags_1,
	sizeof(asn_DEF_DictionaryMsg_tags_1)
		/sizeof(asn_DEF_DictionaryMsg_tags_1[0]), /* 1 */
	asn_DEF_DictionaryMsg_tags_1,	/* Same as above */
	sizeof(asn_DEF_DictionaryMsg_tags_1)
		/sizeof(asn_DEF_DictionaryMsg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DictionaryMsg_1,
	2,	/* Elements count */
	&asn_SPC_DictionaryMsg_specs_1	/* Additional specs */
};

