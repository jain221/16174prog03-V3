/**
 * @file  sensor_data_list.c
 * @brief A struct for maintaining a linked-list of SensorData objects.
 *
 * @note
 * Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
 */




/*******************************************************************************
*                               INCLUDE FILES
*******************************************************************************/
#include "sensor_data_list.h"

#include "alc_assert.h"

#include <stddef.h>




/*******************************************************************************
*                               LOCAL DEFINES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONSTANTS
*******************************************************************************/




/*******************************************************************************
*                               LOCAL DATA TYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL TABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL GLOBAL VARIABLES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTION PROTOTYPES
*******************************************************************************/




/*******************************************************************************
*                               LOCAL CONFIGURATION ERRORS
*******************************************************************************/




/*******************************************************************************
*                               GLOBAL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
void SensorDataList_init(SensorDataList *p_self)
{
    if(p_self)
    {
        p_self->p_first = NULL;
        p_self->p_last  = NULL;
        p_self->size    = 0U;
    }
}
/******************************************************************************/
bool SensorDataList_is_empty(SensorDataList const *p_self)
{
    bool is_empty=false;

    if(p_self)
    {
        if( p_self->size == 0U )
        {
            ALC_ASSERT( p_self->p_first == NULL );
            ALC_ASSERT( p_self->p_last  == NULL );

            is_empty = true;
        }
        else
        {
            ALC_ASSERT( p_self->p_first != NULL );
            ALC_ASSERT( p_self->p_last  != NULL );
        }
    }
    return is_empty;
}
/******************************************************************************/
uint32_t SensorDataList_get_size(SensorDataList const *p_self)
{
    if(p_self)
    {
        return p_self->size;
    }

    return 0U;
}
/******************************************************************************/
uint32_t SensorDataList_max_pop_len(SensorDataList const *p_self, uint32_t limit)
{
    uint32_t count = 0U;

    if( (p_self) && (p_self->p_first) && ( limit > 0U ) )
    {
        for(struct SensorData const* iter=p_self->p_first; iter!=NULL; iter=iter->p_next)
        {
            count++;

            if(
                    ( count >= limit ) ||
                    (
                            ( iter->p_next ) &&
                            ( iter->p_next->seq32 != ( iter->seq32 + 1 ) )
                    )
            )
            {
                // reached limit, OR found gap in sequence numbers
                break;
            }
        }
    }

    return count;
}
/******************************************************************************/
struct SensorData* SensorDataList_pop_front(SensorDataList *p_self)
{
    struct SensorData *p_data=NULL;

    if(p_self)
    {
        if( p_self->size == 0U )
        {
            /* pool is empty */
            ALC_ASSERT( p_self->p_first == NULL );
            ALC_ASSERT( p_self->p_last  == NULL );
        }
        else if( p_self->size == 1U )
        {
            /* pool has 1 entry */
            ALC_ASSERT( p_self->p_first != NULL );
            ALC_ASSERT( p_self->p_first == p_self->p_last );

            p_data = p_self->p_first;

            /* pool is now empty */
            p_self->p_first = NULL;
            p_self->p_last  = NULL;
            p_self->size    = 0U;

            /* Next pointer should be NULL */
            ALC_ASSERT( p_data->p_next == NULL);
        }
        else
        {
            /* pool has more than 1 entry */
            ALC_ASSERT( p_self->p_first != NULL );
            ALC_ASSERT( p_self->p_last  != NULL );
            ALC_ASSERT( p_self->p_first != p_self->p_last );

            p_data = p_self->p_first;

            /* unlink */
            p_self->p_first = p_data->p_next;
            p_self->p_first->p_prev = NULL;
            p_self->size--;
        }

        if(p_data)
        {
            ALC_ASSERT( p_data->p_prev == NULL);

            /* Set the previous/next pointers to NULL */
            p_data->p_prev = NULL;
            p_data->p_next = NULL;
        }
    }

    return p_data;
}
/******************************************************************************/
static inline bool is_greater__(struct SensorData const *p1, struct SensorData const *p2)
{
    return ( ( (int32_t) ( p1->seq32 - p2->seq32 ) ) > 0 );
}
/******************************************************************************/
static inline bool is_equal__(struct SensorData const *p1, struct SensorData const *p2)
{
    return ( p1->seq32 == p2->seq32 );
}
/******************************************************************************/
bool SensorDataList_insert(SensorDataList *p_self, struct SensorData *p_data)
{
    bool inserted=false;

    if( (p_self) && (p_data) )
    {
        /* Ensure next pointer is NULL */
        ALC_ASSERT( p_data->p_prev == NULL );
        ALC_ASSERT( p_data->p_next == NULL );

        if( p_self->size == 0U )
        {
            /* The list is empty */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first == NULL );
            ALC_ASSERT( p_self->p_last  == NULL );

            /* set data as the only entry */
            p_self->p_first = p_data;
            p_self->p_last  = p_data;
            p_self->size    = 1;

            inserted = true;
        }
        else
        {
            /* The list is not empty */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first != NULL );
            ALC_ASSERT( p_self->p_last  != NULL );
            if( p_self->size == 1U )
            {
                ALC_ASSERT( p_self->p_first == p_self->p_last );
            }
            else
            {
                ALC_ASSERT( p_self->p_first != p_self->p_last );
            }

            /* do insert */
            if( is_greater__(p_data, p_self->p_last) )
            {
                /* Add data to the back of the queue */
                p_data->p_prev         = p_self->p_last;
                p_self->p_last->p_next = p_data;
                p_self->p_last         = p_data;
                p_self->size++;

                // success
                inserted = true;
            }
            else if( is_greater__(p_self->p_first, p_data) )
            {
                /* Add data to the front of the queue */
                p_data->p_next          = p_self->p_first;
                p_self->p_first->p_prev = p_data;
                p_self->p_first         = p_data;
                p_self->size++;

                // success
                inserted = true;
            }
            else
            {
                /* The list is not empty */

                struct SensorData *insert_after=NULL;
                struct SensorData *iter;
                for(iter =  p_self->p_last;
                    iter != NULL;
                    iter =  iter->p_prev)
                {
                    if( is_equal__(iter, p_data) )
                    {
                        // Found matching entry
                        break;
                    }
                    else if( is_greater__(p_data, iter) )
                    {
                        insert_after = iter;
                        break;
                    }
                    else
                    {
                        // do nothing here
                    }
                }


                if(insert_after != NULL)
                {
                    // inserting
                    struct SensorData *insert_before = insert_after->p_next;

                    p_data->p_prev = insert_after;
                    p_data->p_next = insert_before;

                    insert_after->p_next  = p_data;
                    insert_before->p_prev = p_data;

                    // success
                    p_self->size++;
                    inserted = true;
                }
            }
        }
    }

    return inserted;
}
/******************************************************************************/
void SensorDataList_push_back(SensorDataList *p_self, struct SensorData *p_data)
{
    if( (p_self) && (p_data) )
    {
        /* Ensure next pointer is NULL */
        ALC_ASSERT( p_data->p_prev == NULL );
        ALC_ASSERT( p_data->p_next == NULL );

        if( p_self->size == 0U )
        {
            /* The list is empty */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first == NULL );
            ALC_ASSERT( p_self->p_last  == NULL );

            /* set data as the only entry */
            p_self->p_first = p_data;
            p_self->p_last  = p_data;
            p_self->size    = 1;
        }
        else
        {
            /* The list is not empty */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first != NULL );
            if( p_self->size == 1U )
            {
                ALC_ASSERT( p_self->p_first == p_self->p_last );
            }
            else
            {
                ALC_ASSERT( p_self->p_last  != NULL );
                ALC_ASSERT( p_self->p_first != p_self->p_last );
            }

            /* Add data to the end of the list */
            p_data->p_prev         = p_self->p_last;
            p_self->p_last->p_next = p_data;
            p_self->p_last         = p_data;
            p_self->size++;
        }
    }
}
/******************************************************************************/
void SensorDataList_check_links(SensorDataList const *p_self)
{
    if(p_self)
    {
        if( p_self->size == 0U )
        {
            /* The list is empty */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first == NULL );
            ALC_ASSERT( p_self->p_last  == NULL );
        }
        else if( p_self->size == 1U )
        {
            /* The list has 1 element */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first != NULL );
            ALC_ASSERT( p_self->p_first == p_self->p_last );

            ALC_ASSERT( p_self->p_first->p_prev == NULL );
            ALC_ASSERT( p_self->p_first->p_next == NULL );

            ALC_ASSERT( p_self->p_last->p_prev == NULL );
            ALC_ASSERT( p_self->p_last->p_next == NULL );
        }
        else
        {
            /* The list has more than 1 element */

            /* do sanity check on pointers */
            ALC_ASSERT( p_self->p_first != NULL );
            ALC_ASSERT( p_self->p_last  != NULL );
            ALC_ASSERT( p_self->p_first != p_self->p_last );

            ALC_ASSERT( p_self->p_first->p_prev == NULL );
            ALC_ASSERT( p_self->p_first->p_next != NULL );

            ALC_ASSERT( p_self->p_last->p_prev != NULL );
            ALC_ASSERT( p_self->p_last->p_next == NULL );
        }


        /* Traverse the list to check the size. */
        uint32_t count=0U;
        struct SensorData const* last_iter=NULL;
        for(struct SensorData const* iter=p_self->p_first; iter!=NULL; iter=iter->p_next)
        {
            ALC_ASSERT( iter->p_prev == last_iter );

            count++;
            last_iter = iter;

            ALC_ASSERT( count <= p_self->size );
        }

        ALC_ASSERT( p_self->p_last == last_iter );
        ALC_ASSERT( count == p_self->size );
    }
}
/******************************************************************************/




/*******************************************************************************
*                               LOCAL FUNCTIONS
*******************************************************************************/



