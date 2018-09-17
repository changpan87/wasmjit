/* -*-mode:c; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
  Copyright (c) 2018 Rian Hunter

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#include <wasmjit/ast.h>
#include <wasmjit/static_runtime.h>
#include <wasmjit/util.h>

#include <math.h>

#include <wasmjit/emscripten_runtime.h>

#define START_MODULE()				\
	struct StaticModuleInst WASM_MODULE_SYMBOL(CURRENT_MODULE);

#define END_MODULE()
#define START_TABLE_DEFS()
#define END_TABLE_DEFS()
#define START_MEMORY_DEFS()
#define END_MEMORY_DEFS()
#define START_GLOBAL_DEFS()
#define END_GLOBAL_DEFS()
#define START_FUNCTION_DEFS()
#define END_FUNCTION_DEFS()

#include <wasmjit/emscripten_runtime_def.h>

#undef START_MODULE
#undef END_MODULE
#undef DEFINE_WASM_GLOBAL
#undef DEFINE_WASM_FUNCTION
#undef DEFINE_WASM_TABLE
#undef DEFINE_WASM_MEMORY
#undef START_TABLE_DEFS
#undef END_TABLE_DEFS
#undef START_MEMORY_DEFS
#undef END_MEMORY_DEFS
#undef START_GLOBAL_DEFS
#undef END_GLOBAL_DEFS
#undef START_FUNCTION_DEFS
#undef END_FUNCTION_DEFS

#define START_MODULE()

/* NB: this is here because I don't feel like implementing the macro
   magic in START_TABLE_DEFS to only add the env table */
struct TableInst WASM_TABLE_SYMBOL(global, table) = {
	.data = NULL,
	.elemtype = ELEMTYPE_ANYFUNC,
	.length = 0,
	.max = 0,
};
extern struct TableInst WASM_TABLE_SYMBOL(env, table);

#define START_TABLE_DEFS(n)						\
	static struct TableInst *CAT(CURRENT_MODULE, _tables)[] = {	\
		&WASM_TABLE_SYMBOL(CURRENT_MODULE, table),
#define DEFINE_WASM_TABLE(_name, ...)			\
	&WASM_TABLE_SYMBOL(CURRENT_MODULE, _name),
#define END_TABLE_DEFS()			\
	};

#define START_MEMORY_DEFS(n)					\
	static struct MemInst *CAT(CURRENT_MODULE, _mems)[] = {
#define DEFINE_WASM_MEMORY(_name, ...)			\
	&WASM_MEMORY_SYMBOL(CURRENT_MODULE, _name),
#define END_MEMORY_DEFS()			\
	};

#define START_GLOBAL_DEFS(n)						\
	static struct GlobalInst *CAT(CURRENT_MODULE,  _globals)[] = {
#define DEFINE_WASM_GLOBAL(_name, ...)			\
	&WASM_GLOBAL_SYMBOL(CURRENT_MODULE, _name),
#define END_GLOBAL_DEFS()			\
	};

#define START_FUNCTION_DEFS(n)					\
	static struct FuncInst *CAT(CURRENT_MODULE,  _funcs)[] = {
#define DEFINE_WASM_FUNCTION(_name, ...)		\
	&WASM_FUNC_SYMBOL(CURRENT_MODULE, _name),
#define END_FUNCTION_DEFS()			\
		};

struct EmscriptenContext g_emscripten_ctx;

#define END_MODULE()							\
	struct StaticModuleInst WASM_MODULE_SYMBOL(CURRENT_MODULE) = {	\
		.module = {						\
			.funcs = {					\
				.n_elts = ARRAY_LEN(CAT(CURRENT_MODULE, _funcs)), \
				.elts = CAT(CURRENT_MODULE, _funcs),	\
			},						\
			.tables = {					\
				.n_elts = ARRAY_LEN(CAT(CURRENT_MODULE, _tables)), \
				.elts = CAT(CURRENT_MODULE, _tables),	\
			},						\
			.mems = {					\
				.n_elts = ARRAY_LEN(CAT(CURRENT_MODULE, _mems)), \
				.elts = CAT(CURRENT_MODULE, _mems),	\
			},						\
			.globals = {					\
				.n_elts = ARRAY_LEN(CAT(CURRENT_MODULE, _globals)), \
				.elts = CAT(CURRENT_MODULE, _globals),	\
			},						\
			.private_data = &g_emscripten_ctx,		\
		},							\
		.initted = 1,						\
	};

#include <wasmjit/emscripten_runtime_def.h>

extern struct FuncInst WASM_FUNC_SYMBOL(asm, _main);
extern struct FuncInst WASM_FUNC_SYMBOL(asm, stackAlloc);
extern struct FuncInst WASM_FUNC_SYMBOL(asm, ___errno_location) __attribute__((weak));

int main(int argc, char *argv[]) {
	return wasmjit_emscripten_invoke_main(&g_emscripten_ctx,
					      &WASM_MEMORY_SYMBOL(env, memory),
					      &WASM_FUNC_SYMBOL(asm, stackAlloc),
					      &WASM_FUNC_SYMBOL(asm, ___errno_location),
					      &WASM_FUNC_SYMBOL(asm, _main),
					      argc, argv);
}
