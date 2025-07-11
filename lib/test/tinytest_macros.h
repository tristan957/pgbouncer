/* tinytest_macros.h -- Copyright 2009 Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TINYTEST_MACROS_H
#define _TINYTEST_MACROS_H

/* Helpers for defining statement-like macros */
#define TT_STMT_BEGIN do {
#define TT_STMT_END } while (0)

/* Redefine this if your test functions want to abort with something besides
 * "goto end;" */
#ifndef TT_EXIT_TEST_FUNCTION
#define TT_EXIT_TEST_FUNCTION TT_STMT_BEGIN goto end; TT_STMT_END
#endif

/* Redefine this if you want to note success/failure in some different way. */
#ifndef TT_DECLARE
#define TT_DECLARE(prefix, args)                                \
	TT_STMT_BEGIN                                           \
	printf("\n  %s %s:%d: ", prefix, __FILE__, __LINE__);      \
	printf args;                                           \
	TT_STMT_END
#endif

/* Announce a failure.  Args are parenthesized printf args. */
#define TT_GRIPE(args) TT_DECLARE("FAIL", args)

/* Announce a non-failure if we're verbose. */
#define TT_BLATHER(args)                                                \
	TT_STMT_BEGIN                                                   \
	if (_tinytest_get_verbosity() > 1) TT_DECLARE("  OK", args);      \
	TT_STMT_END

#define TT_DIE(args)                                            \
	TT_STMT_BEGIN                                           \
	_tinytest_set_test_failed();                            \
	TT_GRIPE(args);                                         \
	TT_EXIT_TEST_FUNCTION;                                  \
	TT_STMT_END

#define TT_FAIL(args)                           \
	TT_STMT_BEGIN                                           \
	_tinytest_set_test_failed();                            \
	TT_GRIPE(args);                                         \
	TT_STMT_END

/* Fail and abort the current test for the reason in msg */
#define tt_abort_printf(msg) TT_DIE(msg)
#define tt_abort_perror(op) TT_DIE(("%s: %s [%d]", (op), strerror(errno), errno))
#define tt_abort_msg(msg) TT_DIE(("%s", msg))
#define tt_abort() TT_DIE(("%s", "(Failed.)"))

/* Fail but do not abort the current test for the reason in msg. */
#define tt_fail_printf(msg) TT_FAIL(msg)
#define tt_fail_perror(op) TT_FAIL(("%s: %s [%d]", (op), strerror(errno), errno))
#define tt_fail_msg(msg) TT_FAIL(("%s", msg))
#define tt_fail() TT_FAIL(("%s", "(Failed.)"))

/* End the current test, and indicate we are skipping it. */
#define tt_skip()                               \
	TT_STMT_BEGIN                                           \
	_tinytest_set_test_skipped();                           \
	TT_EXIT_TEST_FUNCTION;                                  \
	TT_STMT_END

#define _tt_want(b, msg, fail)                          \
	TT_STMT_BEGIN                                   \
	if (!(b)) {                                     \
		_tinytest_set_test_failed();            \
		TT_GRIPE((msg));                        \
		fail;                                   \
	} else {                                        \
		TT_BLATHER((msg));                      \
	}                                               \
	TT_STMT_END

/* Assert b, but do not stop the test if b fails.  Log msg on failure. */
#define tt_want_msg(b, msg)                     \
	_tt_want(b, msg, );

/* Assert b and stop the test if b fails.  Log msg on failure. */
#define tt_assert_msg(b, msg)                   \
	_tt_want(b, msg, TT_EXIT_TEST_FUNCTION);

/* Assert b, but do not stop the test if b fails. */
#define tt_want(b)   tt_want_msg((b), "want("#b ")")
/* Assert b, and stop the test if b fails. */
#define tt_assert(b) tt_assert_msg((b), "assert("#b ")")

#define tt_assert_test_type(a, b, str_test, type, test, fmt)                 \
	TT_STMT_BEGIN                                                   \
	type _val1 = (type)(a);                                         \
	type _val2 = (type)(b);                                         \
	if (!(test)) {                                                  \
		TT_DIE(("assert(%s): "fmt " vs "fmt,                     \
			str_test, _val1, _val2));                       \
	} else {                                                        \
		TT_BLATHER(("assert(%s): "fmt " vs "fmt,                 \
			    str_test, _val1, _val2));                   \
	}                                                               \
	TT_STMT_END

/* Helper: assert that a op b, when cast to type.  Format the values with
 * printf format fmt on failure. */
#define tt_assert_op_type(a, op, b, type, fmt)                              \
	tt_assert_test_type(a, b,#a " "#op " "#b, type, (_val1 op _val2), fmt)

#define tt_int_op(a, op, b)                       \
	tt_assert_test_type(a, b,#a " "#op " "#b, long, (_val1 op _val2), "%ld")

#define tt_uint_op(a, op, b)                                              \
	tt_assert_test_type(a, b,#a " "#op " "#b, unsigned long,            \
			    (_val1 op _val2), "%lu")
#define tt_ptr_op(a, op, b)                                               \
	tt_assert_test_type(a, b,#a " "#op " "#b, void *,                    \
			    (_val1 op _val2), "%p")

#define tt_str_op(a, op, b)                                               \
	tt_assert_test_type(a, b,#a " "#op " "#b, const char *,             \
			    (strcmp(_val1, _val2) op 0), "<%s>")

#define tt_str_contains_op(a, op, b)                                              \
	tt_assert_test_type(a, b,#a " "#op " "#b, const char *,             \
			    (strstr(_val1, _val2) != NULL), "<%s>")
#endif
