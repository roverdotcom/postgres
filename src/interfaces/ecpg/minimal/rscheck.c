#include <Python.h>
#include "postgres_fe.h"
#include <unistd.h>
#include "getopt_long.h"
#include "extern.h"

#define BUFSIZE (1024 * 64) //64K
/* #define RSCHECK_DEBUG 0 */

void* jinja_resolve(const char*);
void python_error(const char*);

int ret_value = 0;
bool autocommit = false,
	 auto_create_c = false,
	 system_includes = false,
	 force_indicator = true,
	 questionmarks = false,
	 regression_mode = false,
	 auto_prepare = false;

char *output_filename; 

enum COMPAT_MODE compat = ECPG_COMPAT_PGSQL;

struct _include_path *include_paths = NULL;
struct cursor *cur = NULL;
struct typedefs *types = NULL;
struct _defines *defines = NULL;

void python_error(const char* message) {
		fprintf(stderr, message);
        	PyErr_Print();
        	Py_Finalize();
}


void *jinja_resolve(const char* raw_template) {
    	PyObject *pString, *pMod, *pFuncRender, *pRetString;
	PyObject *pTemplateClass;
	char *ret_val=NULL;

    	/* 
		import Jinja2
		t = Template(raw_string)
		ret_val = t.render(None)
	*/
    	Py_Initialize();

    	pString = PyUnicode_FromString("jinja2");
    	pMod = PyImport_Import(pString);
    	Py_DECREF(pString);
    	if (!pMod) {
        	python_error("Error finding module jinja2\n");
        	return NULL;
    	}
	pTemplateClass = PyObject_GetAttrString(pMod, "Template");
	if(NULL == pTemplateClass)
	{
		python_error("Error finding Template Class on jinja2 module\n");
		return NULL;
	}

	PyObject *pArgList = Py_BuildValue("(s)", raw_template);
	PyObject *pTemplate = PyObject_CallObject(pTemplateClass, pArgList);

    	if( NULL == pTemplate )
	{
		python_error("Initializing template failed\n");
		return NULL;

	}
	pFuncRender = PyObject_GetAttrString(pTemplate, "render");
	if(!pFuncRender)
	{
		python_error("Unable to find render method on Template class\n");
		return NULL;
	}
	if(PyCallable_Check(pFuncRender))
	{
		pRetString = PyObject_CallObject(pFuncRender, NULL);
		if(NULL == pRetString )
		{
			python_error("Return value was null\n");
			return NULL;
		}
		Py_DECREF(pFuncRender);
		char *temp_string = PyUnicode_AsUTF8(pRetString);
#ifdef RSCHECK_DEBUG
		fprintf(stderr, "After Calling render on [%s] we get: [%s]\n", raw_template, temp_string);
#endif
		Py_DECREF(pRetString);
		ret_val = mm_strdup(temp_string);
	}

	Py_DECREF(pFuncRender);
	Py_DECREF(pTemplateClass);
	Py_DECREF(pMod);

	return (void *)ret_val;
}

int main(int argc, char **argv)
{
// TODO: Help and other command line options (getopt?)

#ifdef YYDEBUG
	base_yydebug = 1;
#endif
	input_filename = _("stdin");
	output_filename = mm_strdup(_("-")); //HACK HACK HACK. This prevents mmfatal from attempting to unlink the output

	base_yyout = fopen(_("/dev/null"), PG_BINARY_W);
	//base_yyin = stdin;



	char *buffer = mm_alloc(sizeof(char)*BUFSIZE);
	const char *prefix = "EXEC SQL ";
	void *renderedBuffer;

	sprintf(buffer, "%s", prefix);

	int test_read = fread(buffer+strlen(prefix), BUFSIZE-(strlen(prefix)+1), 1, stdin);
	int bytes_read = 0;
	if(test_read == 0)
	{
		renderedBuffer = jinja_resolve(buffer);
		bytes_read = strlen(renderedBuffer);
#ifdef YYDEBUG
		printf("Read %d items and a buffer of length %d\n", test_read, bytes_read);
#endif
		base_yyin=fmemopen(renderedBuffer, bytes_read, "r");

		lex_init();
		base_yyparse();
	}



	if(base_yyout != NULL)
	{
		fclose(base_yyout);
	}

}
