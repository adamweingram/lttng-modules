/* SPDX-License-Identifier: (GPL-2.0-only or LGPL-2.1-only)
 *
 * lttng-context-tid.c
 *
 * LTTng TID context.
 *
 * Copyright (C) 2009-2012 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <lttng-events.h>
#include <wrapper/ringbuffer/frontend_types.h>
#include <wrapper/vmalloc.h>
#include <lttng-tracer.h>

static
size_t tid_get_size(size_t offset)
{
	size_t size = 0;

	size += lib_ring_buffer_align(offset, lttng_alignof(pid_t));
	size += sizeof(pid_t);
	return size;
}

static
void tid_record(struct lttng_ctx_field *field,
		 struct lib_ring_buffer_ctx *ctx,
		 struct lttng_channel *chan)
{
	pid_t tid;

	tid = task_pid_nr(current);
	lib_ring_buffer_align_ctx(ctx, lttng_alignof(tid));
	chan->ops->event_write(ctx, &tid, sizeof(tid));
}

static
void tid_get_value(struct lttng_ctx_field *field,
		struct lttng_probe_ctx *lttng_probe_ctx,
		union lttng_ctx_value *value)
{
	pid_t tid;

	tid = task_pid_nr(current);
	value->s64 = tid;
}

int lttng_add_tid_to_ctx(struct lttng_ctx **ctx)
{
	struct lttng_ctx_field *field;

	field = lttng_append_context(ctx);
	if (!field)
		return -ENOMEM;
	if (lttng_find_context(*ctx, "tid")) {
		lttng_remove_context_field(ctx, field);
		return -EEXIST;
	}
	field->event_field.name = "tid";
	field->event_field.type.atype = atype_integer;
	field->event_field.type.u.integer.size = sizeof(pid_t) * CHAR_BIT;
	field->event_field.type.u.integer.alignment = lttng_alignof(pid_t) * CHAR_BIT;
	field->event_field.type.u.integer.signedness = lttng_is_signed_type(pid_t);
	field->event_field.type.u.integer.reverse_byte_order = 0;
	field->event_field.type.u.integer.base = 10;
	field->event_field.type.u.integer.encoding = lttng_encode_none;
	field->get_size = tid_get_size;
	field->record = tid_record;
	field->get_value = tid_get_value;
	lttng_context_update(*ctx);
	wrapper_vmalloc_sync_all();
	return 0;
}
EXPORT_SYMBOL_GPL(lttng_add_tid_to_ctx);
