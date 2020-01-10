/***********************************************************************
    cutswath.c  -  function to cut a swath of a PNM file for PPA printers
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 1998-2000 by the pnm2ppa project
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define __CUTSWATH_C__

#include "syslog.h"
#include "global.h"
#include "debug.h"
#include "defaults.h"
#include "ppa.h"
#include "image.h"

#include "lang.h"

int b_left, b_right, b_vpos , b_dir ;
int c_left, c_right, c_vpos , c_dir ;

/* sweep_data->direction must be set already */

/* Upon successful completion, sweep_data->image_data and
   sweep_data->nozzle_data have been set to pointers which this routine
   malloc()'d. */

/* Upon successful completion, all members of *sweep_data have been set
   except direction, vertical_pos, and next. */

/* Returns: 0 if unsuccessful
            1 if successful, but with non-printing result (end of page)
            2 if successful, with printing result 
	    3 if unsuccessful, exceeded buffer size */

int
cut_im_black_swath (image_t *image, ppaPrinter_t *printer,
		    int maxlines, ppaSweepData_t *sweep_data)
{
    unsigned char *data[4], *ppa, *place, *maxplace;
    int width8, p_width8;
    int k, i, j, left, right, numlines;
    int left_limit , right_limit , bottom_limit, top_limit;
    int  top_vpos , bottom_vpos, sweep_offset;
    int non_blanklines, pre_blanklines, post_blanklines ;
    int even_non_blanklines, even_pre_blanklines, even_post_blanklines ;
    int odd_non_blanklines, odd_pre_blanklines, odd_post_blanklines ;
    int start_even, start_odd;
    int vpos_shift=600;   /* "vertical position" = line number - vpos_shift */
    BOOLEAN at_bottom ;

    BOOLEAN got_nonblank;
    int horzpos, hp2;
    ppaNozzleData_t nozzles[2];

    /* better have (black) maxlines a multiple of 4;
       color uses half this number, and that must be even !*/

    assert ( !(maxlines %4));

    /* safeguard against the user freeing these */
    for (k = 0; k < gMaxPass; k++)
    {
	sweep_data->image_data[k] = NULL;
    }
    sweep_data->nozzle_data = NULL;

    /* read the data from the input file */
    width8 = (image->width + 7) / 8;
    for (k = 0; k < gMaxPass; k++)
    {
	if ((data[k] = malloc (width8 * maxlines)) == NULL)
	{
	  snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
		   gMessages[E_CS_BADMALLOC]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	    return 0;
	}
    }

    /* prevent weird user settings of the margins
     * from causing segfaults
     */
    left_limit = printer->left_margin / 8;
    if ( left_limit < 0 )
      left_limit = 0;
    right_limit = ( gWidth  - printer->right_margin ) / 8;
    if (right_limit > width8 )
      right_limit = width8;
    bottom_limit = gHeight - printer->bottom_margin ;
    if (bottom_limit > gHeight )
      bottom_limit = gHeight;
    if ( bottom_limit < 0 )
      bottom_limit = 0;
    top_limit = printer->top_margin ;
    if ( top_limit < 0 )
      top_limit = 0 ;
    if ( top_limit > bottom_limit )
      top_limit = bottom_limit ;
    
    /* ignore lines that are above the upper margin */
    while (image->blackCurLine < top_limit)
    {
	if (!im_black_readline (image, data, 0))
	{
	  snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
		    gMessages[E_CS_BADTOPMARG]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	    for (k = 0; k < gMaxPass; k++)
	    {
		free (data[k]);
	    }
	    return 0;
	}
    }

    /* eat all lines that are below the lower margin */
    if (image->blackCurLine >= bottom_limit )
    {
	while (image->blackCurLine < image->height)
	    if (!im_black_readline (image, data, 0))
	    {
	      snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
		       gMessages[E_CS_BADBOTMARG]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

		for (k = 0; k < gMaxPass; k++)
		{
		    free (data[k]);
		}
		return 0;
	    }
	for (k = 0; k < gMaxPass; k++)
	{
	    free (data[k]);
	}
	return 1;
    }

    left = right_limit - 1;
    right = left_limit;

    /* eat all beginning blank lines and then up to maxlines or lower margin */
    got_nonblank = false;
    numlines = 0;
    pre_blanklines = 0;
    post_blanklines = 0;
    sweep_offset = 0;
    at_bottom = false ;
    while (( image->blackCurLine  + pre_blanklines  < bottom_limit)
	   && ((numlines + pre_blanklines + post_blanklines ) < maxlines)
	   && ((!gColorMode) || got_nonblank
	       || ((image->buflines < MAXBUFFLINES)
		   || (image->buftype == bitmap))))
      {
	if (!im_black_readline (image, data, width8 * numlines))
	{
	  snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
		   gMessages[E_CS_BADNEXTLINE]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	  for (k = 0; k < gMaxPass; k++)
	    {
	      free (data[k]);
	    }
	  return 0;
	}
	if (!got_nonblank)
	{
	    for (i = left_limit ; i < right_limit; i++)
	    {
		if (data[0][i])
		{
		    left = i;
		    got_nonblank = true;
		    right = i;
		    for (j = right_limit - 1; j > left ; j--)
		      {
			if (data[0][j])
			  {
			    right = j;
			    break;
			  }
		      }
		    /* begin a new swath;, nonblank pixels occur in
		     * bytes "left" through "right", inclusive,
		     * where
		     * left_limit <= left <= right < right_limit.
		     * This range will be expanded if necessary
		     * as the swath is constructed 
		     */
		    DPRINTF("cut_im_black_swath: begin swath, line %d\n",
			    image->blackCurLine ) ;

		    /* vertical position of top  of swath */
		    top_vpos = image->blackCurLine - vpos_shift ;

		    /* predicted vertical position of bottom of swath */
		    bottom_vpos = top_vpos +   maxlines - 1 ;
		    if  ( bottom_vpos >  bottom_limit - vpos_shift)
		      {
			at_bottom = true;
			bottom_vpos = bottom_limit - vpos_shift ;
			/* make sure we are at least 12 dots below the previous
			   black swath vertical position (this is only
			   a possible problem if we are at the bottom limit)*/
			if (  b_vpos + 12 - bottom_vpos > 0 )
			  {
			    post_blanklines +=  (b_vpos + 12 - bottom_vpos) ;
			    bottom_vpos  += post_blanklines  ;
			  }
		      }
		    
		    DPRINTF("cut_im_black_swath: shifted bottom_limit=%d, bottom_vpos=%d\n",
			    bottom_limit - vpos_shift , bottom_vpos) ;
		    
		    /* sweep_offset is the difference in vertical position
		       of consecutive color and black sweeps.
		       Flashing lights happen on the HP820 if  this
		       is  -3, -2, -1, 1 , 2 or 3! */
		    
		    /* test previous color swath vertical positions */
		    sweep_offset = bottom_vpos - c_vpos ; 
		    
		    if ( ( sweep_offset >= -4 ) && 
			 ( sweep_offset < 4 ) &&
			 ( sweep_offset != 0 || at_bottom) )
		      {
			if ( ( at_bottom ) &&
			     ( c_vpos + 5 - sweep_offset < top_vpos +  maxlines - 1 ))
			  {
			    /* we are at the bottom margin , and can add post_blanklines*/
			    post_blanklines +=  4 - sweep_offset ;
			  }
			else
			  {
			    /* we are in the  middle of the page */
			    if ( ( sweep_offset > 0 ) && ( ! at_bottom ))
			      {
				/* shift swath upward to have same vpos */
				pre_blanklines = sweep_offset ;
			      }
			    else
			      {
				/* shift swath up to have vertical position vpos - 5  */
				pre_blanklines = 5 +  sweep_offset ;
			      }
			  } 
		      }
		    /* will add pre_blanklines blank lines at the top of the swath
		       and post_blanklines at the end of the swath. */
		    break;
		}
	    }
	}
	if (got_nonblank)
	  {
	    int newleft = left, newright = right;
	    
	    /* find left-most nonblank */
	    for (i = left_limit ; i < left; i++)
	      {
		if (data[0][width8 * numlines + i])
		  {
		    newleft = i;
		    break;
		  }
	      }
	    /* find right-most nonblank */
	    for (j = right_limit - 1; j > right; j--)
	      {
		if (data[0][width8 * numlines + j])
		  {
		    newright = j;
		    break;
		  }
	      }
	    numlines++;
	    
	    if (newright < newleft)
	      {
		DPRINTF ("Code error! newleft=%d, newright=%d, left=%d, right=%d\n",
			 newleft, newright, left, right);
		for (k = 0; k < gMaxPass; k++)
		  {
		    free (data[k]);
		  }
		return 0;
	      }
	    
	    /* if the next line might push us over the buffer size, stop here! */
	    /* ignore this test for the 720 right now.  Will add better */
	    /* size-guessing for compressed data in the near future! */
	    if (numlines % 2 == 1 && printer->version != HP7X0)
	      {
		int l = newleft, r = newright, w;
		
		l--;
		r += 3;
		w = r - l;
		
		if ((w + 24) * numlines > printer->bufsize)
		  {
		    numlines--;
		    im_unreadline (image, data[0] + width8 * numlines);
		    break;
		  }
		else
		  {
		    left = newleft;
		    right = newright;
		  }
	      }
	    else
	      {
		left = newleft;
		right = newright;
	      }
	  }
      }
    
    if ((gColorMode) && (image->buflines >= MAXBUFFLINES)
	&& (image->buftype == color))
      {
	DPRINTF ("cut_im_black_swath: exceeding buffer size: buflines=%d, MAX=%d\n",
		 image->buflines, MAXBUFFLINES);
	if ((!got_nonblank))
	  {
	    for (k = 0; k < gMaxPass; k++)
	      {
		free (data[k]);
	      }
	    return 3;
	  }
      }
    
    if ((!got_nonblank))
      {
	/* eat all lines that are below the lower margin */
	if (image->blackCurLine >= bottom_limit )
	  {
	    while (image->blackCurLine < image->height)
	      if (!im_black_readline (image, data, 0))
		{
		  snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
			   gMessages[E_CS_BADBOTMARG]);
		  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

		  for (k = 0; k < gMaxPass; k++)
		    {
		      free (data[k]);
		    }
		  return 0;
		}
	    for (k = 0; k < gMaxPass; k++)
	      {
		free (data[k]);
	      }
	    return 1;
	  }
	for (k = 0; k < gMaxPass; k++)
	  {
	    free (data[k]);
	  }
	return 0;		
	/* error, since didn't get to lower margin, yet blank */
      }
    
    
    /* calculate vertical position */
    sweep_data->vertical_pos = image->blackCurLine ;

    /* subtract that 600 dot adjustment here  */
    sweep_data->vertical_pos -= vpos_shift;

    DPRINTF("cut_im_black_swath: end swath, line %d numlines=%d left=%d right=%d\n",
	    image->blackCurLine, numlines, left, right) ;
    /* width of the swath in bytes */
    
    
    /* calculate number of blank lines added at top and bottom of swath */
    
    non_blanklines = numlines ;
    numlines += pre_blanklines ;
    if ( (numlines + post_blanklines) % 2 )
      post_blanklines++ ;
    numlines += post_blanklines ;
    assert (maxlines >= numlines );
    sweep_data->vertical_pos +=  post_blanklines ;

    even_pre_blanklines   = odd_pre_blanklines  = pre_blanklines  / 2 ;
    even_non_blanklines   = odd_non_blanklines  = non_blanklines  / 2 ;
    even_post_blanklines  = odd_post_blanklines = post_blanklines / 2 ;

    start_even= 0;
    start_odd = 1;

    if ( (pre_blanklines %2 == 0) &&
	 !(non_blanklines % 2 == 0) && 
	 !(post_blanklines % 2 == 0))
      { 
	even_non_blanklines++ ;
	odd_post_blanklines++;
	start_even= 0;
	start_odd = 1;
      }
    if ( !(pre_blanklines %2 == 0) &&
	 !(non_blanklines % 2 == 0) && 
	 (post_blanklines % 2 == 0))
      { 
	even_pre_blanklines++;
	odd_non_blanklines++;
	start_even= 1;
	start_odd = 0;
      }
    if ( !(pre_blanklines %2 == 0) &&
	 (non_blanklines % 2 == 0) && 
	 !(post_blanklines % 2 == 0))
      { 
	even_pre_blanklines++;
	odd_post_blanklines++;
	start_even= 1;
	start_odd = 0;
      }

    
    assert (( even_pre_blanklines + odd_pre_blanklines + 
	      even_non_blanklines + odd_non_blanklines + 
	      even_post_blanklines + odd_post_blanklines  ) == numlines );
    
    if (pre_blanklines)
      {
	DPRINTF("cut_im_black_swath: offset %d; add pre_blanklines=%d lines\n",
		sweep_offset, pre_blanklines);
      }
    if (post_blanklines)
      {
	DPRINTF("cut_im_black_swath: offset %d; add post_blanklines=%d lines\n",
		sweep_offset, post_blanklines);
      }


    /* change sweep params */
    /* ensures at least 4 bytes sweep width */
    right +=3;
    left-- ;
    p_width8 = right - left;

    
    if ((ppa = malloc ((p_width8 + 24) * numlines)) == NULL)
      {
	snprintf(syslog_message,message_size,"cut_im_black_swath(): %s", 
		 gMessages[E_CS_BADPPAMALLOC]);
	wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	for (k = 0; k < gMaxPass; k++)
	  {
	    free (data[k]);
	  }
	return 0;
      }
    
    place = ppa;

    
    /* always left-to-right if gUnimode is true */
    if (gUnimode)
	sweep_data->direction = left_to_right;
    else if ((b_vpos > c_vpos) || (c_vpos > sweep_data->vertical_pos))
      {
	if (b_dir == left_to_right)
	  sweep_data->direction = right_to_left;
	else
	  sweep_data->direction = left_to_right;
      } else {
	if (c_dir == left_to_right)
	  sweep_data->direction = right_to_left;
	else
	  sweep_data->direction = left_to_right;
      }

    DPRINTF("cut_im_black_swath: prev b_left %d b_right %d b_vpos %d b_dir %d\n", 
	    b_left, b_right, b_vpos, b_dir);
    DPRINTF("cut_im_black_swath: cur left %d right %d vpos %d dir %d\n", 
	    left, right, sweep_data->vertical_pos, sweep_data->direction);
    b_left = left;
    b_right = right;
    b_vpos = sweep_data->vertical_pos;
    b_dir = sweep_data->direction;
    DPRINTF("cut_im_black_swath: c_left %d c_right %d c_vpos %d c_dir %d\n", 
	    c_left, c_right, c_vpos, c_dir);
 
    /* place 0's in the first 12 columns */
    memset (place, 0, numlines / 2 * 12);
    place += numlines / 2 * 12;

    if (sweep_data->direction == left_to_right)
	DPRINTF ("black: sweep_data->direction == left_to_right\n");
    else if (sweep_data->direction == right_to_left)
	DPRINTF ("black: sweep_data->direction == right_to_left\n");
    else
	DPRINTF ("black: sweep_data->direction == *** ERROR: NOT SET! \n");

    if (sweep_data->direction == right_to_left)	/* right-to-left */
    {
      for (i = p_width8 + 11; i >= 0; i--)
	  {
	    int x ;
	    if (i >= 12  )
	      {
		x = i - 12;
		if( left + x  < left_limit || left + x >= right_limit ) 
		  {
		    /* never print data outside the limits */
		    memset (place, 0, numlines / 2);
		    place += numlines / 2;
		  }
		else
		  {
		    if ( even_pre_blanklines > 0)
		      {
			/* first pre_blanklines lines are blank */
			memset (place, 0, even_pre_blanklines) ;
			place += even_pre_blanklines ;   
		      }
		    for (j = 0; j < even_non_blanklines; j++)
		      {
			*place++ = data[0][((j * 2) + start_even ) * width8 + left + x];
		      }
		    if ( even_post_blanklines > 0 )
		      {
		      /* last post_blanklines lines are blank */
			memset (place, 0, even_post_blanklines) ;
			place += even_post_blanklines ;
		      }
		  }
	      }
	    else
	      {
		memset (place, 0, numlines / 2);
		place += numlines / 2;
	      }
	    
	    if (i < p_width8)
	    {
	      x = i ;
		if( left + x  < left_limit || left + x >= right_limit ) 
		  {
		    /* never print data outside the limits */
		    memset (place, 0, numlines / 2);
		    place += numlines / 2;
		  }
		else
		  {
		    if ( odd_pre_blanklines > 0)
		      {
			/* first pre_blanklines lines are blank */
			memset (place, 0, odd_pre_blanklines) ;
			place += odd_pre_blanklines ;   
		      }
		    for (j = 0; j < odd_non_blanklines; j++)
		      {
			*place++ = data[0][((j * 2) + start_odd ) * width8 + left + x];
		      }
		    if ( odd_post_blanklines > 0 )
		      {
		      /* last post_blanklines lines are blank */
			memset (place, 0, odd_post_blanklines) ;
			place += odd_post_blanklines ;
		      }
		  }
	    }
	    else
	    {
	      memset (place, 0, numlines / 2);
	      place += numlines / 2;
	    }
	  }
    }
    else			/* sweep_data->direction == left_to_right */
      {
	for (i = 0; i < p_width8 + 12; i++)
	  {
	    int x;
	    if (i < p_width8)
	    {
	      x = i;
		if( left + x  < left_limit || left + x >= right_limit ) 
		  {
		    /* never print data outside the limits */
		    memset (place, 0, numlines / 2);
		    place += numlines / 2;
		  }
		else
		  {
		    if ( odd_pre_blanklines > 0)
		      {
			/* first pre_blanklines lines are blank */
			memset (place, 0, odd_pre_blanklines) ;
			place += odd_pre_blanklines ;   
		      }
		    for (j = 0; j < odd_non_blanklines; j++)
		      {
			*place++ = data[0][((j * 2) + start_odd) * width8 + left + x];
		      }
		    if ( odd_post_blanklines > 0 )
		      {
		      /* last post_blanklines lines are blank */
			memset (place, 0, odd_post_blanklines) ;
			place += odd_post_blanklines ;
		      }
		  }
	    }
	    else
	    {
		memset (place, 0, numlines / 2);
		place += numlines / 2;
	    }
	    
	    if (i >= 12)
	    {
	      x = i - 12;
	      if( left + x  < left_limit || left + x >= right_limit ) 
		{
		  /* never print data outside the limits */
		  memset (place, 0, numlines / 2);
		  place += numlines / 2;
		}
	      else
		  {
		    if ( even_pre_blanklines > 0)
		      {
			/* first pre_blanklines lines are blank */
			memset (place, 0, even_pre_blanklines) ;
			place += even_pre_blanklines ;   
		      }
		    for (j = 0; j < even_non_blanklines; j++)
		      {
			*place++ = data[0][((j * 2) + start_even) * width8 + left + x];
		      }
		    if ( even_post_blanklines > 0 )
		      {
		      /* last post_blanklines lines are blank */
			memset (place, 0, even_post_blanklines) ;
			place += even_post_blanklines ;
		      }
		}
	    }
	    else
	    {
	      memset (place, 0, numlines / 2);
	      place += numlines / 2;
	    }
	  }
      }
    
    /* done with data */
    for (k = 0; k < gMaxPass; k++)
      {
	if (data[k] != NULL)
	{
	    free (data[k]);
	}
    }

    /* place 0's in the last 12 columns */
    memset (place, 0, numlines / 2 * 12);
    place += numlines / 2 * 12;
    maxplace = place;

    /* create sweep data */
    sweep_data->image_data[0] = ppa;
    sweep_data->data_size = maxplace - ppa;
    sweep_data->in_color = false;

    horzpos = left * 8 ;

   horzpos += (sweep_data->direction == left_to_right) ?  1  : 0;      

    if (sweep_data->direction == right_to_left ) 
      horzpos +=  printer->r2l_bw_offset ;/* correct bidirectional shearing  */
    
    hp2 = horzpos + (p_width8 + 24) * 8;


    sweep_data->left_margin = horzpos;
    sweep_data->right_margin = hp2 + printer->marg_diff;
    
    for (i = 0; i < 2; i++)
      {
	nozzles[i].DPI = 600;
	nozzles[i].pins_used_d2 = numlines / 2;
	nozzles[i].unused_pins_p1 = 301 - numlines;
	nozzles[i].first_pin = 1;
	if (i == 0)
	  {
	    nozzles[i].left_margin = horzpos + printer->marg_diff;
	    nozzles[i].right_margin = hp2 + printer->marg_diff;
	    if (sweep_data->direction == right_to_left)
	      nozzles[i].nozzle_delay = 0;
	    else
	      nozzles[i].nozzle_delay = 6;	//6
	  }
	else
	  {
	    nozzles[i].left_margin = horzpos;
	    nozzles[i].right_margin = hp2;
	    if (sweep_data->direction == right_to_left)
	      nozzles[i].nozzle_delay = 2;
	    else
	      nozzles[i].nozzle_delay = 0;
	  }
      }
    
    sweep_data->nozzle_data_size = 2;
    sweep_data->nozzle_data = malloc (sizeof (nozzles));
    if (sweep_data->nozzle_data == NULL)
      return 0;
    memcpy (sweep_data->nozzle_data, nozzles, sizeof (nozzles));
    
    return 2;
}

#define UNUSABLE 1
#define CYAN (0<<1)
#define MAGENTA (1<<1)
#define YELLOW (2<<1)
#define ODD (1<<3)
#define EVEN (0<<3)
#define XSHIFT 4
#define XPOS(x) ((x)<<XSHIFT)


/* number of "special" columns at left of sweep */
/* different versions for left-to-right and right-to-left sweeps  */
static int Right_size = 42;
static int Right_l2r[] = { 
  UNUSABLE,
  UNUSABLE,
  UNUSABLE,
  UNUSABLE,
  CYAN | EVEN | XPOS (0),
  CYAN | ODD | XPOS (0),
  CYAN | EVEN | XPOS (1),
  UNUSABLE,
  UNUSABLE,
  CYAN | ODD | XPOS (1),
  CYAN | EVEN | XPOS (2),
  UNUSABLE,
  UNUSABLE,
  CYAN | ODD | XPOS (2),
  CYAN | EVEN | XPOS (3),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (0),
  CYAN | ODD | XPOS (3),
  CYAN | EVEN | XPOS (4),
  UNUSABLE,
  MAGENTA | ODD | XPOS (0),
  MAGENTA | EVEN | XPOS (1),
  CYAN | ODD | XPOS (4),
  CYAN | EVEN | XPOS (5),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | ODD | XPOS (1),
  MAGENTA | EVEN | XPOS (2),
  CYAN | ODD | XPOS (5),
  CYAN | EVEN | XPOS (6),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | ODD | XPOS (2),
  MAGENTA | EVEN | XPOS (3),
  CYAN | ODD | XPOS (6),
  CYAN | EVEN | XPOS (7),
  UNUSABLE,
  YELLOW | EVEN | XPOS (0),
  MAGENTA | ODD | XPOS (3),
  MAGENTA | EVEN | XPOS (4),
  CYAN | ODD | XPOS (7),
  CYAN | EVEN | XPOS (8)
};
static int Right_r2l[] = { 
  UNUSABLE,
  UNUSABLE,
  UNUSABLE, 
  CYAN | EVEN | XPOS (0),
  UNUSABLE,
  CYAN | EVEN | XPOS (1), 
  CYAN | ODD | XPOS (0),
  UNUSABLE,
  UNUSABLE,
  CYAN | EVEN | XPOS (2),
  CYAN | ODD | XPOS (1),
  UNUSABLE,
  UNUSABLE,
  CYAN | EVEN | XPOS (3),
  CYAN | ODD | XPOS (2),
  MAGENTA | EVEN | XPOS (0),
  UNUSABLE,   
  CYAN | EVEN | XPOS (4),
  CYAN | ODD | XPOS (3),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (1),
  MAGENTA | ODD | XPOS (0),
  CYAN | EVEN | XPOS (5),
  CYAN | ODD | XPOS (4),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | EVEN | XPOS (2),
  MAGENTA | ODD | XPOS (1),
  CYAN | EVEN | XPOS (6),
  CYAN | ODD | XPOS (5),
  UNUSABLE,
  UNUSABLE,
  MAGENTA | EVEN | XPOS (3),
  MAGENTA | ODD | XPOS (2),
  CYAN | EVEN | XPOS (7),
  CYAN | ODD | XPOS (6),
  YELLOW | EVEN | XPOS (0),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (4),
  MAGENTA | ODD | XPOS (3),
  CYAN | EVEN | XPOS (8),
  CYAN | ODD | XPOS (7)
};

/* number of "special" columns at left of sweep */
static int Left_size = 32;
static int Left_l2r[] = { 
  YELLOW | ODD | XPOS (0),
  YELLOW | EVEN | XPOS (0),
  YELLOW | ODD | XPOS (1),
  YELLOW | EVEN | XPOS (1),
  YELLOW | ODD | XPOS (2),
  YELLOW | EVEN | XPOS (2),
  YELLOW | ODD | XPOS (3),
  UNUSABLE,
  MAGENTA | ODD | XPOS (0),
  YELLOW | EVEN | XPOS (3),
  YELLOW | ODD | XPOS (4),
  MAGENTA | EVEN | XPOS (0),
  MAGENTA | ODD | XPOS (1),
  YELLOW | EVEN | XPOS (4),
  YELLOW | ODD | XPOS (5),
  MAGENTA | EVEN | XPOS (1),
  MAGENTA | ODD | XPOS (2),
  YELLOW | EVEN | XPOS (5),
  YELLOW | ODD | XPOS (6),
  UNUSABLE,
  MAGENTA | EVEN | XPOS (2),
  MAGENTA | ODD | XPOS (3),
  YELLOW | EVEN | XPOS (6),
  YELLOW | ODD | XPOS (7),
  UNUSABLE,
  CYAN | ODD | XPOS (0),
  MAGENTA | EVEN | XPOS (3),
  MAGENTA | ODD | XPOS (4),
  YELLOW | EVEN | XPOS (7),
  YELLOW | ODD | XPOS (8),
  CYAN | EVEN | XPOS (0),
  CYAN | ODD | XPOS (1)
};

/* the final odd yellow swing buffer doesnt fit in right-to-left
 * color sweeps.  Instead of redesigning the structure of  the Left of
 * the sweep (If it works, dont fix it ...)
 * I'll just arrange that no printable data ever uses
 * this last position .. (the leftmost printed dot) (duncan)
 */

static int Left_r2l[] = { 
  UNUSABLE,//YELLOW | ODD | XPOS (0),   //this data doesnt fit,  what to do?  
  YELLOW | ODD | XPOS (1),
  YELLOW | EVEN | XPOS (0),
  YELLOW | ODD | XPOS (2),
  YELLOW | EVEN | XPOS (1),
  YELLOW | ODD | XPOS (3),
  YELLOW | EVEN | XPOS (2),
  MAGENTA | ODD | XPOS (0),
  UNUSABLE,
  YELLOW | ODD | XPOS (4),
  YELLOW | EVEN | XPOS (3),
  MAGENTA | ODD | XPOS (1),
  MAGENTA | EVEN | XPOS (0),
  YELLOW | ODD | XPOS (5),
  YELLOW | EVEN | XPOS (4),
  MAGENTA | ODD | XPOS (2),
  MAGENTA | EVEN | XPOS (1),
  YELLOW | ODD | XPOS (6),
  YELLOW | EVEN | XPOS (5),
  UNUSABLE, 
  MAGENTA | ODD | XPOS (3),
  MAGENTA | EVEN | XPOS (2),
  YELLOW | ODD | XPOS (7),
  YELLOW | EVEN | XPOS (6),
  CYAN | ODD | XPOS (0),
  UNUSABLE, 
  MAGENTA | ODD | XPOS (4),
  MAGENTA | EVEN | XPOS (3),
  YELLOW | ODD | XPOS (8),
  YELLOW | EVEN | XPOS (7),
  CYAN | ODD | XPOS (1),
  CYAN | EVEN | XPOS (0)
};

#define GET_USABLE(A,i) (!(A[i] & 0x1))
#define GET_COLOR(A,i) ((A[i] >> 1) & 0x3);
#define GET_ODD(A,i) ((A[i] >> 3) & 0x1);
#define GET_X(A,i) ((A[i] >> 4));

int
cut_im_color_swath (image_t *image, ppaPrinter_t *printer, int maxlines,
		    ppaSweepData_t *sweep_data)
{
  unsigned char *data[4], *ppa, *place, *maxplace;
  int width8, p_width8;
  int k, i, j, left , right, numlines;
  int left_limit, right_limit, bottom_limit, top_limit ;
  int  top_vpos , bottom_vpos, sweep_offset ;
  int non_blanklines, pre_blanklines, post_blanklines ;
  int even_non_blanklines, even_pre_blanklines, even_post_blanklines ;
  int odd_non_blanklines, odd_pre_blanklines, odd_post_blanklines ;
  BOOLEAN at_bottom ;
  int start_even, start_odd;
  int vpos_shift=600;   /* "vertical position" = line number - vpos_shift */
  
  BOOLEAN got_nonblank;
  int horzpos, hp2;
  ppaNozzleData_t nozzles[6];
  
  /* maxlines needs to be even */
  assert( !(maxlines % 2 ));
  
  place = NULL;
  ppa = NULL;
  maxplace = NULL;
  
  /* safeguard against the user freeing these */
  for (k = 0; k < gMaxPass; k++)	//mocho
    {
      sweep_data->image_data[k] = NULL;
    }
  sweep_data->nozzle_data = NULL;
  
  /* read the data from the input file */
  width8 = (image->width / 2 + 7) / 8;
  DPRINTF ("width8 * 3 * maxlines = %d\n", width8 * 3 * maxlines);
  for (k = 0; k < gMaxPass; k++)	//mocho
    {
      if ((data[k] = malloc (width8 * 3 * maxlines + 8)) == NULL)
	{
	  snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
		   gMessages[E_CS_BADMALLOC]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	  return 0;
	}
    }
  
    /* protect against weird margin settings */
    
    left_limit =  printer->left_margin / 2 / 8;
    if ( left_limit < 0 ) 
      left_limit = 0 ;
    right_limit = (gWidth - printer->right_margin) / 2 / 8;
    if ( right_limit > width8  )
      right_limit =  width8 ;
    bottom_limit = gHeight - printer->bottom_margin  ;
    if ( bottom_limit > gHeight)
      bottom_limit = gHeight ;
    if ( bottom_limit < 0 )
      bottom_limit = 0;
    top_limit = printer->top_margin ;
    if ( top_limit < 0 )
      top_limit = 0 ;
    if ( top_limit > bottom_limit )
      top_limit = bottom_limit ;


    /* ignore lines that are above the upper margin */
    while (image->colorCurLine < top_limit )
      if (!im_color_readline (image, data, 0))
	{
	  snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
		   gMessages[E_CS_BADTOPMARG]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	  
	  for (k = 0; k < gMaxPass; k++)	// mocho
	    {
	      free (data[k]);
	    }
	  return 0;
	}
    
    /* eat all lines that are below the lower margin */
    if (image->colorCurLine >= bottom_limit )                 
      {
	while (image->colorCurLine < image->height)
	  if (!im_color_readline (image, data, 0))
	    {
	      snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
		       gMessages[E_CS_BADBOTMARG]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	      for (k = 0; k < gMaxPass; k++)
		{
		  free (data[k]);	//mocho
		}
	      return 0;
	    }
	for (k = 0; k < gMaxPass; k++)
	  {
	    free (data[k]);	//mocho
	  }
	DPRINTF (" cut_im_color_swath: return 1 on line %d\n", __LINE__);
	return 1;
      }
    
    
    left = right_limit - 1; 
    right = left_limit;
    
    got_nonblank = false;
    numlines = 0;
    pre_blanklines = 0;
    post_blanklines = 0;
    at_bottom = false ;
    sweep_offset = 0;
    /* eat all beginning blank lines and then up to maxlines or lower margin */
    while (
	   ( image->colorCurLine + (2* pre_blanklines) < bottom_limit ) &&
	   ( (numlines + pre_blanklines + post_blanklines) < maxlines)
	   && (got_nonblank ||
	       ((image->buflines < MAXBUFFLINES) || (image->buftype == color))))
      {
	if (!im_color_readline (image, data, width8 * 3 * numlines))	//mocho
	  {
	    snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
		     gMessages[E_CS_BADNEXTLINE]);
	    wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	    for (k = 0; k < gMaxPass; k++)
	      {
		free (data[k]);	//mocho
	      }
	    return 0;
	  }
	if (!got_nonblank)
	  {
	    for (i = left_limit * 3; i < right_limit * 3; i++)
	      {
		if ((data[gMaxPass - 1][i]))
		  {
		    left = i / 3;
		    got_nonblank = true;
		    right = left;
		    for (j = right_limit * 3 - 1; j > left * 3 + 2; j--)
		      {
			if ((data[gMaxPass - 1][j]))
			  {
			    right = j / 3;
			    break;
			  }
		      }
		    /* begin a new swath;, nonblank pixels occur in
		     * bytes "left" through "right", inclusive,
		     * where
		     * left_limit <= left <= right < right_limit.
		     * This range will be expanded if necessary
		     * as the swath is constructed 
		     */
		    DPRINTF("cut_im_color_swath: begin swath, line %d\n",
			    image->colorCurLine ) ;
		    
		    /* vertical position of top  of swath */
		    top_vpos = image->colorCurLine - vpos_shift + printer->ColBwOffsY  ;
		    
		    /* predicted vertical position of bottom of swath */
		    bottom_vpos = top_vpos +   2* ( maxlines - 1);
		    if ( bottom_vpos >  bottom_limit - vpos_shift + printer->ColBwOffsY )
		      {
			at_bottom = true;
			bottom_vpos = bottom_limit - vpos_shift + printer->ColBwOffsY ;
			/* make sure we are at least 20 dots below the previous
			   color swath vertical position (this is only
			   a possible problem if we are at the bottom limit)*/
			if ( ( c_vpos + 21 - bottom_vpos )/2 > 0)
			  {
			    post_blanklines += (c_vpos + 21  - bottom_vpos) /2 ;
			    bottom_vpos +=  2 * post_blanklines  ;
			  }
		      }
		    
		    DPRINTF("cut_im_color_swath: shifted bottom_limit=%d, bottom_vpos=%d\n",
			    bottom_limit - vpos_shift, bottom_vpos) ;

		    /* sweep_offset is the difference in vertical position
		       of consecutive color and black sweeps.
		       Flashing lights happen on the HP820 if  this
		       is  -3, -2, -1, 1 , 2 or 3! */

		    /* compare to  last black vpos */
		    sweep_offset = bottom_vpos - b_vpos ; 
		    
		    if ( ( sweep_offset >= -4 )  && 
			 ( sweep_offset <  4 )  &&
			 ( (sweep_offset !=0 )  || at_bottom ))
		      {
			if ( (at_bottom) &&
			     ( b_vpos + 2 * ((5 - sweep_offset ) / 2) < top_vpos + 2 * ( maxlines - 1 ) ) )
			  {
				/* we are at bottom of page, and can add post_blanklines */
			    post_blanklines  +=  ( 5 - sweep_offset ) /2 ;
			  }
			else 
			  {
				/* we are in the middle of the page ;
				   add pre_blanklines to shift swath end up */
			    if (  sweep_offset == 3 )
			      {
				/* shift swath upward to  vpos - 7  */
				pre_blanklines = 5;
			      }
			    else if ( sweep_offset == 2 )
			      {
				/* shift swath upward to  vpos - 6  */
				pre_blanklines = 4;
			      }
			    else if ( sweep_offset == 1 )
			      {
				/* shift swath upward to  vpos - 7 */
				pre_blanklines = 4;
			      }
			    else if ( sweep_offset == -1 )
			      {
				/* shift swath upward to vpos -7 */
				pre_blanklines = 3;
			      }
			    else if ( sweep_offset == -2 )
			      {
				/* shift swath upward to vpos -6 */
				pre_blanklines = 2;
			      }
			    else if ( sweep_offset == -3 )
			      {
				/* shift swath upward to  vpos - 7 */
				pre_blanklines = 2;
			      }
			    else if ( sweep_offset == -4 )
			      {
				/* shift swath upward to  vpos - 6 */
				pre_blanklines = 1;
			      }			    
			  }
 		      }
		    /*will add pre_blanklines blank lines at the top of the swath
		      and post_blanklines at the end of the swath. 
		      Note that allowance has been made for a possible
		      extra post_blankline to compensate for odd numlines */
		    break;
		}
	    }
	}
	
	if (got_nonblank)
	  {
	    /* find left-most nonblank */
	    for (i = left_limit * 3; i < left * 3; i++)
	      {
		// fprintf(stderr, "left: %d ", i);
		if ((data[gMaxPass - 1][width8 * 3 * numlines + i]))
		  {
		    left = i / 3;
		    break;
		}
	    }
	    /* find right-most nonblank */
	    for (j =  right_limit * 3 - 1 ;  j > right * 3 + 2 ; j--)
	      {
		// fprintf(stderr, "right: %d ", i);
		if ((data[gMaxPass - 1][width8 * 3 * numlines + j]))
		  {
		    right = j / 3;
		    break;
		  }
	      }
	    numlines++;
	}
    }

    if ((image->buflines >= MAXBUFFLINES) && (image->buftype == bitmap))
    {
	DPRINTF
	    ("cut_im_color_swath: exceeding buffer size: image.buflines=%d, MAX=%d\n",
	     image->buflines, MAXBUFFLINES);
	if ((!got_nonblank))
	{
	    for (k = 0; k < gMaxPass; k++)
		free (data[k]);
	    return 3;
	}
    }


    if ((!got_nonblank))
    {
	/* eat all lines that are below the lower margin */
	if (image->colorCurLine >= bottom_limit)
	  {
	    while (image->colorCurLine < image->height)
		if (!im_color_readline (image, data, 0))
		{
		  snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
			   gMessages[E_CS_BADBOTMARG]);
		  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

		  for (k = 0; k < gMaxPass; k++)
		    {
		      free (data[k]);	//mocho
		    }
		  return 0;
		}
	    for (k = 0; k < gMaxPass; k++)
	      {
		free (data[k]);	//mocho
	      }
	    DPRINTF
	      ("cut_im_color_swath: return 1 on line %d; ccl: %d; height: %d\n",
	       __LINE__, image->colorCurLine, bottom_limit );
	    return 1;
	  }
	for (k = 0; k < gMaxPass; k++)
	  {
	    free (data[k]);	//mocho
	  }
	return 0;		
	/* error, since didn't get to lower margin, yet blank */
    }
    
    sweep_data->vertical_pos = image->colorCurLine + printer->ColBwOffsY ;

    /* subtract that 600 dot adjustment here  */
    sweep_data->vertical_pos -= vpos_shift;

    DPRINTF("cut_im_color_swath: end swath, line %d numlines=%d left=%d right=%d\n",
	image->colorCurLine, numlines, left, right ) ;



    /* calculate number of blank lines added at top and bottom of swath */

    non_blanklines = numlines ;
    numlines += pre_blanklines ;
    if ( (numlines + post_blanklines) % 2 )
      post_blanklines++ ;
    numlines += post_blanklines ;
    assert (maxlines >= numlines );
    sweep_data->vertical_pos +=  2 * post_blanklines ;


    even_pre_blanklines   = odd_pre_blanklines  = pre_blanklines  / 2 ;
    even_non_blanklines   = odd_non_blanklines  = non_blanklines  / 2 ;
    even_post_blanklines  = odd_post_blanklines = post_blanklines / 2 ;

    start_even= 0;
    start_odd = 1;

    if ( (pre_blanklines %2 == 0) &&
	 !(non_blanklines % 2 == 0) && 
	 !(post_blanklines % 2 == 0))
      { 
	even_non_blanklines++ ;
	odd_post_blanklines++;
	start_even= 0;
	start_odd = 1;
      }
    if ( !(pre_blanklines %2 == 0) &&
	 !(non_blanklines % 2 == 0) && 
	 (post_blanklines % 2 == 0))
      { 
	even_pre_blanklines++;
	odd_non_blanklines++;
	start_even= 1;
	start_odd = 0;
      }
    if ( !(pre_blanklines %2 == 0) &&
	 (non_blanklines % 2 == 0) && 
	 !(post_blanklines % 2 == 0))
      { 
	even_pre_blanklines++;
	odd_post_blanklines++;
	start_even= 1;
	start_odd = 0;
      }

     
    assert ( numlines % 2 == 0);
    assert (( even_pre_blanklines +  
	      even_non_blanklines  + 
	      even_post_blanklines  ) == numlines/2 );
    assert (( odd_pre_blanklines + 
	      odd_non_blanklines + 
	      odd_post_blanklines  ) == numlines/2 );
    

    if (pre_blanklines)
      {
	DPRINTF("cut_im_color_swath: offset %d; add pre_blanklines=%d lines\n",
		sweep_offset, pre_blanklines);
      }
    if (post_blanklines)
      {
	DPRINTF("cut_im_color_swath: offset %d; add post_blanklines=%d lines\n",
		sweep_offset, post_blanklines);
      }


       /* printing in the leftmost and rightmost bytes of the color
       * swath leads to problems, so expand the swath width to avoid
       * using these locations.  Nothing will ever get printed in these
       * expanded  ends of the swath.   (They are either whitespace,
       * or outside the limits, where data is rejected below)
       */
       left -= 2;
       right += 3;
       p_width8 = right - left ;

       /* enforce a minimum swath width of 8 bytes */
       if ( p_width8 < 8 ) 
	 {
	   if  (left > left_limit + 8 )
	     left = left - 8 + p_width8 ;
	   else
	     right = right + 8 - p_width8 ;
	 }
       p_width8 = right - left ;
       
    /* always left-to-right if gUnimode is true */
    if (gUnimode)
	sweep_data->direction = left_to_right;
    else if ((b_vpos > c_vpos) && (b_vpos < sweep_data->vertical_pos))
      {
	if (b_dir == left_to_right)
	  sweep_data->direction = right_to_left;
	else
	  sweep_data->direction = left_to_right;
      } else 
	{
	if (c_dir == left_to_right)
	  sweep_data->direction = right_to_left;
	else
	  sweep_data->direction = left_to_right;
	}

    DPRINTF("cut_im_color_swath: b_left %d b_right %d b_vpos %d b_dir %d\n", 
	    b_left, b_right, b_vpos, b_dir);
    DPRINTF("cut_im_color_swath: prev c_left %d c_right %d c_vpos %d c_dir %d\n", 
	    c_left, c_right, c_vpos, c_dir);
    DPRINTF("cut_im_color_swath: cur left %d right %d vpos %d dir %d\n", 
	    left, right, sweep_data->vertical_pos, sweep_data->direction);
    c_left = left;
    c_right = right;
    c_vpos = sweep_data->vertical_pos;
    c_dir = sweep_data->direction;

    
    for (k = 0; k < gMaxPass; k++)
    {
	if ((ppa = malloc ( (p_width8 + 3 ) * numlines * 3)) == NULL)
	{
	  snprintf(syslog_message,message_size,"cut_im_color_swath(): %s", 
		   gMessages[E_CS_BADPPAMALLOC]);
	  wrap_syslog (LOG_CRIT,"%s",syslog_message); 

	  for (k = 0; k < gMaxPass; k++)
	    free (data[k]);
	  return 0;
	}

	place = ppa;

	if (sweep_data->direction == right_to_left)
	{
	  for (i = (p_width8 + 1) * 6  - 1 ; i >= 0; i--)
	    {
	      int color, x, odd, y ;
	      j = (p_width8 + 1) * 6  - 1 - i ;
	      if (i < Left_size )
		{
		  if (!GET_USABLE (Left_r2l, i))
		    {
		      memset (place, 0x00, numlines / 2);
		      place += numlines / 2;
		      continue;
		    }
		  odd = GET_ODD (Left_r2l, i  );
		  color = GET_COLOR (Left_r2l, i);
		  x = GET_X (Left_r2l, i );
		}
	      else if (j < Right_size )
		{
		  if (!GET_USABLE (Right_r2l,j))
		    {
		      memset (place, 0x00, numlines / 2);
		      place += numlines / 2;
		      continue;
		    }
		  color = GET_COLOR (Right_r2l, j);
		  odd = GET_ODD (Right_r2l, j);
		  x = p_width8  - 3 - GET_X (Right_r2l, j);
		}
	      else
		{
		  color = (i / 2 - 15) % 3;
		  odd = (i + 1) % 2;
		  x = (i - 24) / 6 - 1 + odd + (color == 0 ? 0 :/* cyan */
						color == 1 ? 4 :/* magenta */
						8);	         /* yellow */
		}
		if (x + left < left_limit || x + left >= right_limit)
		  {
		    /* never print data that is outside the limits */
		    memset (place, 0x00, numlines / 2);
		    place += numlines / 2;
		  }
		else
		  {
		    int new_pre_blanklines , new_non_blanklines , new_post_blanklines;
		    int new_start;
		    if (odd) 
		      {
			/* odd lines */
			new_pre_blanklines  = odd_pre_blanklines  ;
			new_non_blanklines  = odd_non_blanklines  ;
			new_post_blanklines = odd_post_blanklines ;
			new_start = start_odd;
		      }
		    else
		      {
			/* even lines */
			new_pre_blanklines  = even_pre_blanklines  ;
			new_non_blanklines  = even_non_blanklines  ;
			new_post_blanklines = even_post_blanklines ;
			new_start = start_even;
		      }
		    if ( new_pre_blanklines > 0 )
		      {
			/* first pre_blanklines lines are blank */
			memset(place, 0x00, new_pre_blanklines ) ;
			place += new_pre_blanklines ;
		      }
		    for (y = 0; y < new_non_blanklines ; y++)
		      {
			int index ;
			index = ( (y * 2 + new_start) * width8 * 3 + 
				  (x + left ) * 3 + color ) ;
			
			if (index >= (width8 * 3 * maxlines ))
			  {
			    DPRINTF ("index out of range! index = %d\n", index);
			    index = (width8 * 3 * maxlines ) - 1;
			  }
			
			assert ( index  < width8 * 3 * maxlines   && index >= 0 );
			*place++ = data[k][index];
		      }
		    if ( new_post_blanklines > 0 )
		      {
			/* last post_blanklines lines are blank */
			memset(place, 0x00, new_post_blanklines ) ;
			place += new_post_blanklines  ;
		      }
		  }
	    }
	    
	}
	else   /* sweep direction is left-to-right  */
	  {
	    for (i = 0; i < (p_width8 + 1) * 6 ; i++)
	    {
		int color, x, odd, y;
		j = (p_width8 + 1) * 6  - 1 - i ;
		if (i <  Left_size)
		{
		    if (!GET_USABLE (Left_l2r, i))
		    {
			memset (place, 0x00, numlines / 2);
			place += numlines / 2;
			continue;
		    }
		    color = GET_COLOR (Left_l2r, i);
		    odd = GET_ODD (Left_l2r, i);
		    x = GET_X (Left_l2r, i);
		}
		else if ( j < Right_size )
		{
		    if (!GET_USABLE (Right_l2r, j))
		    {
			memset (place, 0x00, numlines / 2);
			place += numlines / 2;
			continue;
		    }
		    color = GET_COLOR (Right_l2r, j);
		    odd = GET_ODD (Right_l2r, j);
		    x = p_width8  -3 - GET_X (Right_l2r,j);
		}
		else
		{
		    color = (i / 2 - 15) % 3;
		    odd = i % 2;
		    x = (i - 24) / 6 - 1 + odd + (color == 0 ? 0 :/* cyan */
						  color == 1 ? 4 :/* magenta */
						  8);	          /* yellow */
		}
		if (x + left < left_limit || x + left >= right_limit)
		  {
		    /* never print data that is outside the limits */
		    memset (place, 0x00, numlines / 2);
		    place += numlines / 2;
		  }
		else
		  {
		    int new_pre_blanklines , new_non_blanklines , new_post_blanklines;
		    int new_start;
		    if (odd) 
		      {
			/* odd lines */
			new_pre_blanklines  = odd_pre_blanklines  ;
			new_non_blanklines  = odd_non_blanklines  ;
			new_post_blanklines = odd_post_blanklines ;
			new_start = start_odd;
		      }
		    else
		      {
			/* even lines */
			new_pre_blanklines  = even_pre_blanklines  ;
			new_non_blanklines  = even_non_blanklines  ;
			new_post_blanklines = even_post_blanklines ;
			new_start = start_even;
		      }
		    if ( new_pre_blanklines > 0 )
		      {
			/* first pre_blanklines lines are blank */
			memset (place, 0x00, new_pre_blanklines ) ;
			place += new_pre_blanklines  ;
		      }
		    for (y = 0; y < new_non_blanklines; y++)
		      {
			int index ;
			index = ((y * 2 + new_start) * width8 * 3 +
				 (x + left ) * 3 + color);
			
			if (index >= (width8 * 3 * maxlines + 8))
			  {
			    DPRINTF ("index out of range! index = %d\n",index);
			    index = (width8 * 3 * maxlines + 8) - 1;
			  }
			
			assert ( index < width8 * 3 * maxlines + 8  && index >= 0 );
			*place++ = data[k][index];
		      }
		    if ( new_post_blanklines > 0 )
		      {
			/* last post_blanklines lines are blank */
			memset(place, 0x00, new_post_blanklines) ;
			place += new_post_blanklines ;
		      }
		  }
	    }
	  }
	maxplace = place;

	/* create sweep data */
	sweep_data->image_data[k] = ppa;
    }

    sweep_data->data_size = maxplace - ppa;
    sweep_data->in_color = true;

    horzpos = (left - 7) * 8  * 2 + printer->ColBwOffsX - 600 ;

     horzpos += (sweep_data->direction == left_to_right) ? 2 : 12 ;	

    if (sweep_data->direction == right_to_left ) 
      horzpos +=   printer->r2l_col_offset  ;  /*correct bidirectional shearing*/
 
    hp2 = 16 + horzpos + p_width8 * 2 * 8;
    sweep_data->left_margin = horzpos;
    sweep_data->right_margin = hp2 + 0x74;	/* printer->color_marg_diff */

    DPRINTF("sweep data: left_margin = %d, right margin = %d\n",
	sweep_data->left_margin, sweep_data->right_margin);

    for (i = 0; i < 6; i++)
    {
	nozzles[i].DPI = 300;
	nozzles[i].pins_used_d2 = numlines / 2;
	nozzles[i].unused_pins_p1 = 65 - numlines;
	nozzles[i].first_pin = 1;
	if (i == 0)
	{
	    nozzles[i].left_margin = horzpos + 0x74;
	    nozzles[i].right_margin = hp2 + 0x74;
	    if (sweep_data->direction == right_to_left)
		nozzles[i].nozzle_delay = 0;
	    else
		nozzles[i].nozzle_delay = 0;
	}
	if (i == 1)
	{
	    nozzles[i].left_margin = horzpos + 0x64;
	    nozzles[i].right_margin = hp2 + 0x64;
	    if (sweep_data->direction == right_to_left)
		nozzles[i].nozzle_delay = 0;
	    else
		nozzles[i].nozzle_delay = 0;
	}
	if (i == 2)
	{
	    nozzles[i].left_margin = horzpos + 0x3A;
	    nozzles[i].right_margin = hp2 + 0x3A;
	    if (sweep_data->direction == right_to_left)
		nozzles[i].nozzle_delay = 0x0A;
	    else
		nozzles[i].nozzle_delay = 0x0A;
	}
	if (i == 3)
	{
	    nozzles[i].left_margin = horzpos + 0x3A;
	    nozzles[i].right_margin = hp2 + 0x3A;
	    if (sweep_data->direction == right_to_left)
		nozzles[i].nozzle_delay = 0x0A;
	    else
		nozzles[i].nozzle_delay = 0x0A;

	}
	if (i == 4)
	{
	    nozzles[i].left_margin = horzpos + 0x10;
	    nozzles[i].right_margin = hp2 + 0x10;
	    if (sweep_data->direction == right_to_left)
		nozzles[i].nozzle_delay = 0x04;
	    else
		nozzles[i].nozzle_delay = 0x04;

	}
	if (i == 5)
	{
	    nozzles[i].left_margin = horzpos + 0x00;
	    nozzles[i].right_margin = hp2 + 0x00;
	    if (sweep_data->direction == right_to_left)
		nozzles[i].nozzle_delay = 0x04;
	    else
		nozzles[i].nozzle_delay = 0x04;

	}
    }

    sweep_data->nozzle_data_size = 6;
    sweep_data->nozzle_data = malloc (sizeof (nozzles));
    if (sweep_data->nozzle_data == NULL)
	return 0;
    memcpy (sweep_data->nozzle_data, nozzles, sizeof (nozzles));

    DPRINTF ("cut_im_color_swath: return 2 on line %d\n", __LINE__);

    for (k = 0; k < gMaxPass; k++)
	free (data[k]);

    return 2;

}

/*

  Here, we should call cut_im_black_swath and cut_im_color_swath to begin.

  Then, we need to determine which of these swaths comes earlier on the page
  and print it first.  Then we should re-call the appropriate cut_im_*_swath
  function to refill that buffer and repeat this procedure.

*/


int 
ppa_print_page (ppaPrinter_t *printer, image_t *image) 
{
    ppaSweepData_t sweeps[3], *curblack, *curcolor, *prev, *tmp;
    int reload[2] = { 1, 1 };
    int done[2] = { 0, 0 };
    int k,found[2];
    int firstfound;
    int color_maxlines=64, black_maxlines=300;
    

    /* note: a full-size (all nozzles) color swath is 64 even lines (300dpi); 
       a full-size (all nozzles) black swath is 300 lines (150 even + 150 odd);
       is is necessary to restrict black swath to 128 lines (64 even + 64 odd)
       on some printer models, when printing in dual inks, color+black ? 
       HP7x0:   NO, but there is some artifact at the begining of
                each black swath that does not immediately follow a black 
		swath.  (see the calibrate_ppa --align  output....)
		so leave the setting at 128 in gColorMode for now. 
		(duncan, 2000-10-24)
       HP820:  YES flashes lights ... (maybe  it will work if what
               produces the artifacts on HP7x0 is fixed..?)
       HP1000: ?
    */

    switch (printer->version)
      { 
      case HP7X0:
	black_maxlines=300;
	color_maxlines=64;
	if (gColorMode)
	  black_maxlines = color_maxlines * 2 ;
	break;
      case HP820:
	black_maxlines=300;
	color_maxlines=64;
	if (gColorMode)
	  black_maxlines = color_maxlines * 2 ;
	break;
      case HP1000:
	black_maxlines=300;
	color_maxlines=64;
	if (gColorMode)
	  black_maxlines = color_maxlines * 2 ;
	break;
      default:
	break;
      }

    DPRINTF("ppa_print_page(): black_maxlines=%d, color_blacklines=%d\n",
	    black_maxlines, color_maxlines );

    /* initialize vertical position markers, etc. */
    c_vpos = b_vpos = 0;
    
    firstfound=0;

    curblack = &sweeps[0];
    curcolor = &sweeps[1];

    prev = NULL;

    DPRINTF ("ppa_print_page: chkpt 1\n");



    /* note: the bidirectional print sweep code is not
     quite perfect as not every  right-to-left sweep gets
     followed by a left-to-right sweep.   However, the empirical
     choices below for the initialization of
     sweeps[2].direction seem to make the opposite direction
     sweeps come after each other, most of the time.  Different
     choices were needed in  gColorMode and !gColorMode... (duncan)
    */

    
    sweeps[0].direction = left_to_right;
    sweeps[0].next = NULL;
    sweeps[1].direction = left_to_right;
    sweeps[1].next = NULL;
    sweeps[2].direction = right_to_left;
    sweeps[2].next = NULL;

    if ( !gColorMode)
      {
	done[1] = 1;
	sweeps[0].direction = right_to_left;
	sweeps[1].direction = left_to_right;
	sweeps[2].direction = left_to_right;
      }

    sweeps[1].next = NULL;
    found[0]=found[1]=0;

    while (!done[0] || !done[1])
    {  
      
      DPRINTF("ppa_print_page: statistics follows\n");
      DPRINTF("done[0]=%d\tdone[1]=%d\nreload[0]=%d\treload[1]=%d\nfound[0]=%d\tfound[1]=%d\n", 
	      done[0],done[1],reload[0],reload[1],found[0],found[1]);
 
      if (reload[0])
	{
	  reload[0] = 0;

	  switch (cut_im_black_swath (image, printer, black_maxlines, curblack))
	    {
	    case 0:
	      snprintf(syslog_message,message_size,"ppa_print_page(): %s cut_im_black_swath()", 
		       gMessages[E_CS_ERROR]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	      return 1;
	      break;
	    case 1:
		   done[0] = 1;
		   break;
	    case 2:
		   found[0]=1;
		   if (!firstfound)
		     {
		       firstfound=1;
		       curcolor->direction=right_to_left;
		     }
		   break;
	    case 3: 
	      reload[0]=1;
	      found[0]=0;
	      break;
	    default:
	      snprintf(syslog_message,message_size,"ppa_print_page(): %s", 
		       gMessages[E_CS_BADSWITCH]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	      return 1;
	    }
	}
      if (reload[1] && !done[1])
	{
	  reload[1] = 0;
	  switch (cut_im_color_swath (image, printer, color_maxlines, curcolor))
	    {
	    case 0:
	      snprintf(syslog_message,message_size, "ppa_print_page(): %s cut_im_color_swath()", 
		       gMessages[E_CS_ERROR]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	      break;
	      return 1;
	    case 1:
		   DPRINTF("ppa_print_page: cut_im_color_swath returned 1\n");
		   done[1] = 1;
		   break;
	    case 2:
		   found[1]=1;
		    if (!firstfound)
		     {
		       firstfound=1;
		       curblack->direction=right_to_left;
		     }
		    break;
	    case 3:
		   reload[1]=1;
		   found[1]=0;
		   break;
	    default:
	      snprintf(syslog_message,message_size,"ppa_print_page(): %s ",  
		       gMessages[E_CS_BADSWITCH]);
	      wrap_syslog (LOG_CRIT,"%s",syslog_message); 
	      return 1;
	    }
	}

      if (done[0] && done[1])
	break;

      if ( (!done[0] && found[0] && 
	   (done[1] || !found[1] || curblack->vertical_pos < curcolor->vertical_pos)) )
      
	{
	  DPRINTF("ppa_print_page: chkpt 2\n");

	    reload[0] = 1;
	    DPRINTF("ppa_print_page: chkpt 3\n");
	    if (prev)
	    {
		prev->next = curblack;
		DPRINTF("ppa_print_sweep 1\n");
		ppa_print_sweep (printer, prev);
		for (k = 0; k < gMaxPass; k++)
		    free (prev->image_data[k]);
		free (prev->nozzle_data);
	    }
	    else
		prev = &sweeps[2];

	    /* move the current sweep into the 'prev' position */
	    tmp = curblack;
	    curblack = prev;
	    prev = tmp;
	}
    	else
	  if ( (!done[1] && found[1]) ) // &&  (done[0] || !found[0] ||curblack->vertical_pos > curcolor->vertical_pos)))
	{
	  DPRINTF("ppa_print_page: chkpt 4 \n");
	      
	    reload[1] = 1;
	    
	    if (prev)
	    {
		prev->next = curcolor;
		DPRINTF("ppa_print_sweep 2\n");
		ppa_print_sweep (printer, prev);
		for (k = 0; k < gMaxPass; k++)
		    free (prev->image_data[k]);
		free (prev->nozzle_data);
	    }
	    else
		prev = &sweeps[2];

	    tmp = curcolor;
	    curcolor = prev;
	    prev = tmp;
	}
    }

    DPRINTF ("ppa_print_page: chkpt 5\n");

    if (prev)
    {
	DPRINTF ("ppa_print_page: chkpt 6 about to clear prev\n");

	prev->next = NULL;
	DPRINTF ("ppa_print_sweep 3\n");
	ppa_print_sweep (printer, prev);
	for (k = 0; k < gMaxPass; k++)
	    free (prev->image_data[k]);
	free (prev->nozzle_data);
	DPRINTF ("ppa_print_page: chkpt 7 cleared prev\n");
    }

    DPRINTF ("ppa_print_page: chkpt 8 Finish.\n");
    
    for (k=0; k < gMaxPass; k++)
	    free(image->buffer[k]);

    return 0;
}
