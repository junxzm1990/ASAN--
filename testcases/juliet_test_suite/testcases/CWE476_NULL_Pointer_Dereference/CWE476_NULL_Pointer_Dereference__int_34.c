/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE476_NULL_Pointer_Dereference__int_34.c
Label Definition File: CWE476_NULL_Pointer_Dereference.label.xml
Template File: sources-sinks-34.tmpl.c
*/
/*
 * @description
 * CWE: 476 NULL Pointer Dereference
 * BadSource:  Set data to NULL
 * GoodSource: Initialize data
 * Sinks:
 *    GoodSink: Check for NULL before attempting to print data
 *    BadSink : Print data
 * Flow Variant: 34 Data flow: use of a union containing two methods of accessing the same data (within the same function)
 *
 * */

#include "std_testcase.h"

#include <wchar.h>

typedef union
{
    int * unionFirst;
    int * unionSecond;
} CWE476_NULL_Pointer_Dereference__int_34_unionType;

#ifndef OMITBAD

void CWE476_NULL_Pointer_Dereference__int_34_bad()
{
    int * data;
    CWE476_NULL_Pointer_Dereference__int_34_unionType myUnion;
    /* POTENTIAL FLAW: Set data to NULL */
    data = NULL;
    myUnion.unionFirst = data;
    {
        int * data = myUnion.unionSecond;
        /* POTENTIAL FLAW: Attempt to use data, which may be NULL */
        printIntLine(*data);
    }
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* goodG2B() uses the GoodSource with the BadSink */
static void goodG2B()
{
    int * data;
    CWE476_NULL_Pointer_Dereference__int_34_unionType myUnion;
    /* FIX: Initialize data */
    {
        int tmpData = 5;
        data = &tmpData;
    }
    myUnion.unionFirst = data;
    {
        int * data = myUnion.unionSecond;
        /* POTENTIAL FLAW: Attempt to use data, which may be NULL */
        printIntLine(*data);
    }
}

/* goodB2G() uses the BadSource with the GoodSink */
static void goodB2G()
{
    int * data;
    CWE476_NULL_Pointer_Dereference__int_34_unionType myUnion;
    /* POTENTIAL FLAW: Set data to NULL */
    data = NULL;
    myUnion.unionFirst = data;
    {
        int * data = myUnion.unionSecond;
        /* FIX: Check for NULL before attempting to print data */
        if (data != NULL)
        {
            printIntLine(*data);
        }
        else
        {
            printLine("data is NULL");
        }
    }
}

void CWE476_NULL_Pointer_Dereference__int_34_good()
{
    goodG2B();
    goodB2G();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
   its own for testing or for building a binary to use in testing binary
   analysis tools. It is not used when compiling all the testcases as one
   application, which is how source code analysis tools are tested. */
#ifdef INCLUDEMAIN

int main(int argc, char * argv[])
{
    /* seed randomness */
    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    printLine("Calling good()...");
    CWE476_NULL_Pointer_Dereference__int_34_good();
    printLine("Finished good()");
#endif /* OMITGOOD */
#ifndef OMITBAD
    printLine("Calling bad()...");
    CWE476_NULL_Pointer_Dereference__int_34_bad();
    printLine("Finished bad()");
#endif /* OMITBAD */
    return 0;
}

#endif
