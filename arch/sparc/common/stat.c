/*
 * =====================================================================================
 *
 *       Filename:  stat.c
 *
 *    Description:  This file implements all the means for the statistics
 *    gathering
 *
 *        Version:  1.0
 *        Created:  01/10/08 14:06:41
 *       Modified:  01/10/08 14:06:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


#include "types.h"
#include "stat.h"

#include <strings.h>
#include <time.h>


static int stat_fini = False;

struct _stat statistics;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  STAT_reset
 *  Description:  This function resets all the statistics
 * =====================================================================================
 */
void STAT_reset(void)
{
    /*  reset the statistics vector */
    bzero((void*)&statistics, sizeof(statistics));

    statistics.starttime = time(NULL);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  STAT_show
 *  Description:  This functions collects the statistics in a file. The file is
 *  called stats.log and it is placed in the current folder.
 *  It is not possible to call this function without calling STAT_fini()
 *  function before.
 *
 *  @see STAT_fini()
 * =====================================================================================
 */
void STAT_show(void)
{
    if( stat_fini )
    {
        /*  FIXME: to be implemented    */
        printf("nothing yet\n");
    }
    else
        printf("STATS not finished\n");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  STAT_fini
 *  Description:  This function finalizes the STAT structure calculating the
 *  total simulation time.
 *  It is not possible to collect the statistics without calling this function
 *
 *  @see STAT_show()
 * =====================================================================================
 */
void STAT_fini(void)
{
    statistics.tottime = time(NULL) - statistics.starttime;

    stat_fini = True;
}

