/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83_bad.cpp
Label Definition File: CWE121_Stack_Based_Buffer_Overflow__CWE131.label.xml
Template File: sources-sink-83_bad.tmpl.cpp
*/
/*
 * @description
 * CWE: 121 Stack Based Buffer Overflow
 * BadSource:  Allocate memory without using sizeof(int)
 * GoodSource: Allocate memory using sizeof(int)
 * Sinks: loop
 *    BadSink : Copy array to data using a loop
 * Flow Variant: 83 Data flow: data passed to class constructor and destructor by declaring the class object on the stack
 *
 * */
#ifndef OMITBAD

#include "std_testcase.h"
#include "CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83.h"

namespace CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83
{
CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83_bad::CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83_bad(int * dataCopy)
{
    data = dataCopy;
    /* FLAW: Allocate memory without using sizeof(int) */
    data = (int *)ALLOCA(10);
}

CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83_bad::~CWE121_Stack_Based_Buffer_Overflow__CWE131_loop_83_bad()
{
    {
        int source[10] = {0};
        size_t i;
        /* POTENTIAL FLAW: Possible buffer overflow if data was not allocated correctly in the source */
        for (i = 0; i < 10; i++)
        {
            data[i] = source[i];
        }
        printIntLine(data[0]);
    }
}
}
#endif /* OMITBAD */
