/*		Interface to hypertext object		HText.c
**		=============================
*/

#include <ctype.h>
#include "HTUtils.h"
#include "HTString.h"
#include "HText.h"
#include "HyperText.h"

/*	Exports
*/ 
PUBLIC HText * HTMainText = 0;		/* Equivalent of main window */
PUBLIC HTParentAnchor * HTMainAnchor = 0;	/* Anchor for HTMainText */


/*			Creation Method
**			---------------
*/
PUBLIC HText *	HText_new ARGS1(HTParentAnchor *,anchor)
{
    HTLine * line;
    HText * self = (HText *) malloc(sizeof(*self));
    if (!self) return self;
    
    if (!loaded_texts) loaded_texts = HTList_new();
    HTList_addObject(loaded_texts, self);
    if (HTList_count(loaded_texts) >= LOADED_LIMIT) {
        if (TRACE) fprintf(stderr, "GridText: Freeing off cached doc.\n"); 
        HText_free((HText *)HTList_removeFirstObject(loaded_texts));
    }
    
    line = self->last_line = (HTLine *)malloc(LINE_SIZE(MAX_LINE));
    line->next = line->prev = line;
    line->offset = line->size = 0;
    self->lines = self->chars = 0;
    self->title = 0;
    self->first_anchor = self->last_anchor = 0;
    self->style = &default_style;
    self->top_of_screen = 0;
    self->node_anchor = anchor;
    self->last_anchor_number = 0;	/* Numbering of them for references */
    self->stale = YES;
    
    HTAnchor_setDocument(anchor, (HyperDoc *)self);

    clear_screen();
    HTMainText = self;
    HTMainAnchor = anchor;
    self->display_on_the_fly = DISPLAY_LINES;
    
    if (!space_string) {	/* Make a blank line */
        char *p;
	space_string = (char *)malloc(HTScreenWidth+1);
        for (p=space_string; p<space_string+HTScreenWidth; p++) 
            *p = ' '; 		/* Used for printfs later */
        space_string[HTScreenWidth] = '\0'; 
    }
    
    return self;
}


/*	Free Entire Text
**	----------------
*/
PUBLIC void 	HText_free ARGS1(HText *,self)
{
    HTAnchor_setDocument(self->node_anchor, (HyperDoc *)0);
    
    while(YES) {		/* Free off line array */
        HTLine * l = self->last_line;
	l->next->prev = l->prev;
	l->prev->next = l->next;	/* Unlink l */
	self->last_line = l->prev;
	free(l);
	if (l == self->last_line) break;	/* empty */
    };
    
    while(self->first_anchor) {		/* Free off anchor array */
        TextAnchor * l = self->first_anchor;
	self->first_anchor = l->next;
	free(l);
    }
    free(self);
}

/*		Display Methods
**		---------------
*/
/*	Clear the screen (on screen-mode systems)
**	----------------
*/
PUBLIC void clear_screen NOARGS
{
#ifdef VM
#ifdef NEWLIB
    int newlib_ncmd = 2;
    char newlib_cmd[2][80];

    memset(newlib_cmd, ' ', sizeof(newlib_cmd));
    strncpy(newlib_cmd[0], "clear", 5);		/* Clear screen */
    strncpy(newlib_cmd[1], "quit", 4);		/* Leave newlib */
    newlib(newlib_cmd, &newlib_ncmd);		/* Execute command */
    
#else		/* not NEWLIB - real VM */
    system("CLRSCRN");				/* Clear screen */
#endif		/* not NEWLIB */
#else		/* Not VM */
	/* Do nothing */
#endif		/* Not VM */
    if (HTMainText) HTMainText->stale = YES;
}


/*	Output a line
**	-------------
*/
PRIVATE void display_line ARGS1(HTLine *,line)
{
   printf("%s%s\n", SPACES(line->offset), line->data);
   
}

/*	Output the title line
**	---------------------
*/
PRIVATE void display_title ARGS1(HText *,text)
{
    CONST char * title = HTAnchor_title(text->node_anchor);
    char percent[20], format[20];
    if (text->lines > (DISPLAY_LINES-1)) {
#ifdef NOPE
	sprintf(percent, " (p%d of %d)",
	    (text->top_of_screen/(DISPLAY_LINES-1)) + 1,
	    (text->lines-1)/(DISPLAY_LINES-1) + 1);
	sprintf(percent, " (%d%%)",
	    100*(text->top_of_screen+DISPLAY_LINES-1)/	/* Seen */
	    	(text->lines));				/* Total */
#else
	sprintf(percent, " (%d/%d)",
	    text->top_of_screen+DISPLAY_LINES-1,	/* Seen */
	    text->lines);				/* Total */
#endif
    } else {
	percent[0] = 0;	/* Null string */
    }

    sprintf(format, "%%%d.%ds%%s\n",	/* Generate format string */
		    HTScreenWidth-strlen(percent),
		    HTScreenWidth-strlen(percent));
    if (TRACE) fprintf(stderr, "FORMAT IS `%s'\n", format);
    printf(format, title, percent);
}


/*	Fill the screen with blank after the file
**	--------------------------
*/
PRIVATE void fill_screen ARGS1(int,n)
{
    if (n<=0) return;
    if (!interactive) return;
#ifndef VIOLA    
    printf("%s\n", end_mark);
    n--;
    
    for (; n; n--) {
        printf("\n");
    }
#endif
}


/*	Output a page
**	-------------
*/
PRIVATE void display_page ARGS2(HText *,text, int,line_number)
{
    HTLine * line = text->last_line->prev;
    int i;
    CONST char * title = HTAnchor_title(text->node_anchor);
    int lines_of_text = title ? DISPLAY_LINES-1 : DISPLAY_LINES;
    int last_screen = text->lines - lines_of_text;

/*	Constrain the line number to be within the document
*/
    if (text->lines <= lines_of_text) line_number = 0;
    else if (line_number>last_screen) line_number = last_screen;
    else if (line_number < 0) line_number = 0;
    
    for(i=0,  line = text->last_line->next;		/* Find line */
    	i<line_number && (line!=text->last_line);
	i++, line=line->next) /* Loop */;

    while((line!=text->last_line) && (line->size==0)) {	/* Skip blank lines */
        line = line->next;
        line_number ++;
    }

/*	Can we just scroll to it?
*/ 
    if (!text->stale && (line_number>=text->top_of_screen) &&
    	(line_number < text->top_of_screen + DISPLAY_LINES)) {	/* Yes */
	lines_of_text = line_number - text->top_of_screen;
	line = text->next_line;
        text->top_of_screen = line_number;
    } else {
	clear_screen();						/* No */
        text->top_of_screen = line_number;
	if (title) display_title(text);
    }
    
    
 /*	print it
 */
    if (line) {
      for(i=0;
	  i< lines_of_text && (line != text->last_line); i++)  {
        display_line(line);
	line = line->next;
      }
      fill_screen(lines_of_text - i);
    }

    text->next_line = line;	/* Line after screen */
    text->stale = NO;		/* Display is up-to-date */
}


/*			Object Building methods
**			-----------------------
**
**	These are used by a parser to build the text in an object
*/
PUBLIC void HText_beginAppend ARGS1(HText *,text)
{
    text->permissible_split = 0;
    text->in_line_1 = YES;
}


/*	Add a new line of text
**	----------------------
**
** On entry,
**
**	split	is zero for newline function, else number of characters
**		before split.
**	text->display_on_the_fly
**		may be set to indicate direct output of the finished line.
** On exit,
**		A new line has been made, justified according to the
**		current style. Text after the split (if split nonzero)
**		is taken over onto the next line.
**
**		If display_on_the_fly is set, then it is decremented and
**		the finished line is displayed.
*/
#define new_line(text) split_line(text, 0)

PRIVATE void split_line ARGS2(HText *,text, int,split)
{
    HTStyle * style = text->style;
    int spare;
    int indent = text->in_line_1 ? text->style->indent1st
    				 : text->style->leftIndent;
    
/*	Make new line
*/
    HTLine * line = (HTLine *) malloc(LINE_SIZE(MAX_LINE));
    HTLine * previous = text->last_line;
    
    text->lines++;
    
    previous->next->prev = line;
    line->prev = previous;
    line->next = previous->next;
    previous->next = line;
    text->last_line = line;
    line->size = 0;
    line->offset = 0;

/*	Split at required point
*/    
    if (split) {	/* Delete space at "split" splitting line */
	char * p;
	previous->data[previous->size] = 0;
	for (p = &previous->data[split]; *p; p++)
	    if (*p != ' ') break;
	strcpy(line->data, p);
	line->size = strlen(line->data);
	previous->size = split;
    }
    
/*	Economise on space
*/
    
    while ((previous->size > 0) &&
    	(previous->data[previous->size-1] == ' '))	/* Strip trailers */
        previous->size--;
	
    previous = (HTLine *) realloc(previous, LINE_SIZE(previous->size));
    previous->prev->next = previous;
    previous->next->prev = previous;	/* Could be same node of course */

/*	Terminate finished line for printing
*/
    previous->data[previous->size] = 0;
     
    
/*	Align left, right or center
*/

    spare =  HTScreenWidth -
    		style->rightIndent + style->leftIndent -
    		previous->size;	/* @@ first line indent */
		
    switch (style->alignment) {
	case HT_CENTER :
	    previous->offset = previous->offset + indent + spare/2;
	    break;
	case HT_RIGHT :
	    previous->offset = previous->offset + indent + spare;
	    break;
	case HT_LEFT :
	case HT_JUSTIFY :		/* Not implemented */
	default:
	    previous->offset = previous->offset + indent;
	    break;
    } /* switch */

    text->chars = text->chars + previous->size + 1;	/* 1 for the line */
    text->in_line_1 = NO;		/* unless caller sets it otherwise */
    
/*	If displaying as we go, output it:
*/
    if (text->display_on_the_fly) {
        if ((text->display_on_the_fly == DISPLAY_LINES) &&
		    HTAnchor_title(text->node_anchor)) { /* First line */
	    display_title(text);
	    text->display_on_the_fly--;
	}
	display_line(previous);
	text->display_on_the_fly--;
    }
} /* split_line */


/*	Allow vertical blank space
**	--------------------------
*/
PRIVATE void blank_lines ARGS2(HText *,text, int,newlines)
{
    if (text->last_line->size == 0) {	/* No text on current line */
	HTLine * line = text->last_line->prev;
	while ((line!=text->last_line) && (line->size == 0)) {
	    if (newlines==0) break;
	    newlines--;		/* Don't bother: already blank */
	    line = line->prev;
	}
    } else {
	newlines++;			/* Need also to finish this line */
    }

    for(;newlines;newlines--) {
	new_line(text);
    }
    text->in_line_1 = YES;
}


/*	New paragraph in current style
**	------------------------------
** See also: setStyle.
*/

PUBLIC void HText_appendParagraph ARGS1(HText *,text)
{
    int after = text->style->spaceAfter;
    int before = text->style->spaceBefore;
    blank_lines(text, after>before ? after : before);
}


/*	Set Style
**	---------
**
**	Does not filter unnecessary style changes.
*/
PUBLIC void HText_setStyle ARGS2(HText *,text, HTStyle *,style)
{
    int after, before;

    if (!style) return;				/* Safety */
    after = text->style->spaceAfter;
    before = style->spaceBefore;
    if (TRACE) fprintf(stderr, "HTML: Change to style %s\n", style->name);

    blank_lines (text, after>before ? after : before);

    text->style = style;
}


/*	Append a character to the text object
**	-------------------------------------
*/
PUBLIC void HText_appendCharacter ARGS2(HText *,text, char,ch)
{
    HTLine * line = text->last_line;
    HTStyle * style = text->style;
    int indent = text->in_line_1 ? style->indent1st : style->leftIndent;
    
/*		New Line
*/
    if (ch == '\n') {
	    new_line(text);
	    text->in_line_1 = YES;	/* First line of new paragraph */
	    return;
    }

/* 		Tabs
*/

    if (ch == '\t') {
        HTTabStop * tab;
	int target;	/* Where to tab to */
	int here = line->size + line->offset +indent;
        if (style->tabs) {	/* Use tab table */
	    for (tab = style->tabs;
	    	tab->position <= line->size;
		tab++)
		if (!tab->position) {
		    new_line(text);
		    return;
		}
	    target = tab->position;
	} else if (text->in_line_1) {	/* Use 2nd indent */
	    if (here >= style->leftIndent) {
	        new_line(text); /* wrap */
		return;
	    } else {
	        target = style->leftIndent;
	    }
	} else {		/* Default tabs align with left indent mod 8 */
	    target = ((line->offset + line->size + 8) & (-8))
	    		+ style->leftIndent;
	}
	if (target > HTScreenWidth - style->rightIndent) {
	    new_line(text);
	    return;
	} else {
            text->permissible_split = line->size;	/* Can split here */
	    if (line->size == 0) line->offset = line->offset + target - here;
	    else for(; here<target; here++) {
                line->data[line->size++] = ' ';	/* Put character into line */
	    }
	    return;
	}
	/*NOTREACHED*/
    } /* if tab */ 

    
    if (ch==' ') {
        text->permissible_split = line->size;	/* Can split here */
    }

/*	Check for end of line
*/    
    if (indent + line->offset + line->size + style->rightIndent
    		>= HTScreenWidth) {
        if (style->wordWrap) {
	    split_line(text, text->permissible_split);
	    if (ch==' ') return;	/* Ignore space causing split */
	} else new_line(text);
    }

/*	Insert normal characters
*/
    if (ch == HT_NON_BREAK_SPACE) {
        ch = ' ';
    }
    {
        HTLine * line = text->last_line;	/* May have changed */
        HTFont font = style->font;
        line->data[line->size++] =	/* Put character into line */
           font & HT_CAPITALS ? TOUPPER(ch) : ch;
        if (font & HT_DOUBLE)		/* Do again if doubled */
            HText_appendCharacter(text, HT_NON_BREAK_SPACE);
	    /* NOT a permissible split */ 
    }
}

/*		Anchor handling
**		---------------
*/
/*	Start an anchor field
*/
PUBLIC void HText_beginAnchor ARGS2(HText *,text, HTChildAnchor *,anc)
{
    TextAnchor * a = (TextAnchor *) malloc(sizeof(*a));
    
    a->start = text->chars + text->last_line->size;
    a->extent = 0;
    if (text->last_anchor) {
        text->last_anchor->next = a;
    } else {
        text->first_anchor = a;
    }
    a->next = 0;
    a->anchor = anc;
    text->last_anchor = a;
    
    if (HTAnchor_followMainLink((HTAnchor*)anc)) {
        a->number = ++(text->last_anchor_number);
    } else {
        a->number = 0;
    }
}


PUBLIC void HText_endAnchor ARGS1(HText *,text)
{
    TextAnchor * a = text->last_anchor;
    char marker[100];
    if (a->number && display_anchors) {	 /* If it goes somewhere */
	sprintf(marker, reference_mark, a->number);
	HText_appendText(text, marker);
    }
    a->extent = text->chars + text->last_line->size - a->start;
}


PUBLIC void HText_appendText ARGS2(HText *,text, CONST char *,str)
{
    CONST char * p;
    for(p=str; *p; p++) {
        HText_appendCharacter(text, *p);
    }
}


PUBLIC void HText_endAppend ARGS1(HText *,text)
{
    new_line(text);
    
    if (text->display_on_the_fly) {
        fill_screen(text->display_on_the_fly);
	text->display_on_the_fly = 0;
	text->stale = NO;
    }
}



/* 	Dump diagnostics to stderr
*/
PUBLIC void HText_dump ARGS1(HText *,text)
{
    fprintf(stderr, "HText: Dump called\n");
}
	

/*	Return the anchor associated with this node
*/
PUBLIC HTParentAnchor * HText_nodeAnchor ARGS1(HText *,text)
{
    return text->node_anchor;
}

/*				GridText specials
**				=================
*/
/*	Return the anchor with index N
**
**	The index corresponds to the number we print in the anchor.
*/
PUBLIC HTChildAnchor * HText_childNumber ARGS2(HText *,text, int,number)
{
    TextAnchor * a;
    for (a = text->first_anchor; a; a = a->next) {
        if (a->number == number) return a->anchor;
    }
    return (HTChildAnchor *)0;	/* Fail */
}

PUBLIC void HText_setStale ARGS1(HText *,text)
{
    text->stale = YES;
}

PUBLIC void HText_refresh ARGS1(HText *,text)
{
    if (text->stale) display_page(text, text->top_of_screen);
}

PUBLIC int HText_sourceAnchors ARGS1(HText *,text)
{
    return text->last_anchor_number;
}

PUBLIC BOOL HText_canScrollUp ARGS1(HText *,text)
{
    return text->top_of_screen != 0;
}

PUBLIC BOOL HText_canScrollDown ARGS1(HText *,text)
{
    char * title = text->title;
    return (text->top_of_screen + DISPLAY_LINES - (title? 1:0) ) <
    		text->lines;
}

/*		Scroll actions
*/
PUBLIC void HText_scrollTop ARGS1(HText *,text)
{
    display_page(text, 0);
}

PUBLIC void HText_scrollDown ARGS1(HText *,text)
{
    display_page(text, text->top_of_screen + DISPLAY_LINES -1);
}

PUBLIC void HText_scrollUp ARGS1(HText *,text)
{
    display_page(text, text->top_of_screen - DISPLAY_LINES +1);
}

PUBLIC void HText_scrollBottom ARGS1(HText *,text)
{
}


/*		Browsing functions
**		==================
*/

/* Bring to front and highlight it
*/

PRIVATE int line_for_char ARGS2(HText *,text, int,char_num)
{
    int line_number =0;
    int characters = 0;
    HTLine * line = text->last_line->next;
    for(;;) {
	if (line == text->last_line) return 0;	/* Invalid */
        characters = characters + line->size + 1;
	if (characters > char_num) return line_number;
	line_number ++;
	line = line->next;
    }
}

PUBLIC BOOL HText_select ARGS1(HText *,text)
{
    if (text != HTMainText) {
        HTMainText = text;
	HTMainAnchor = text->node_anchor;
	display_page(text, text->top_of_screen);
    }
    return YES;
}

PUBLIC BOOL HText_selectAnchor ARGS2(HText *,text, HTChildAnchor *,anchor)
{
    TextAnchor * a;

/* This is done later, hence HText_select is unused in GridText.c
   Should it be the contrary ? @@@
    if (text != HTMainText) {
        HText_select(text);
    }
*/

    for(a=text->first_anchor; a; a=a->next) {
        if (a->anchor == anchor) break;
    }
    if (!a) {
        if (TRACE) fprintf(stderr, "HText: No such anchor in this text!\n");
        return NO;
    }
/*
    if (text != HTMainText) {
        HTMainText = text;
	HTMainAnchor = text->node_anchor;
    }
*/
    {
	 int l = line_for_char(text, a->start);
	if (TRACE) fprintf(stderr,
	    "HText: Selecting anchor [%d] at character %d, line %d\n",
	    a->number, a->start, l);
	 if ((l >= text->top_of_screen) &&
	     ( l < text->top_of_screen + DISPLAY_LINES-1))
	     l = text->top_of_screen;	/* Anchor already visible */
	 else l = l - (DISPLAY_LINES/3);		/* Scroll to it */
        display_page(text, l);
    }
    
    return YES;
}
 

/*		Editing functions		- NOT IMPLEMENTED
**		=================
**
**	These are called from the application. There are many more functions
**	not included here from the orginal text object.
*/

/*	Style handling:
*/
/*	Apply this style to the selection
*/
PUBLIC void HText_applyStyle ARGS2(HText *, me, HTStyle *,style)
{
    
}


/*	Update all text with changed style.
*/
PUBLIC void HText_updateStyle ARGS2(HText *, me, HTStyle *,style)
{
    
}


/*	Return style of  selection
*/
PUBLIC HTStyle * HText_selectionStyle ARGS2(
	HText *,me,
	HTStyleSheet *,sheet)
{
    return 0;
}


/*	Paste in styled text
*/
PUBLIC void HText_replaceSel ARGS3(
	HText *,me,
	CONST char *,aString, 
	HTStyle *,aStyle)
{
}


/*	Apply this style to the selection and all similarly formatted text
**	(style recovery only)
*/
PUBLIC void HTextApplyToSimilar ARGS2(HText *,me, HTStyle *,style)
{
    
}

 
/*	Select the first unstyled run.
**	(style recovery only)
*/
PUBLIC void HTextSelectUnstyled ARGS2(HText *,me, HTStyleSheet *,sheet)
{
    
}


/*	Anchor handling:
*/
PUBLIC void		HText_unlinkSelection ARGS1(HText *,me)
{
    
}

PUBLIC HTAnchor *	HText_referenceSelected ARGS1(HText *,me)
{
     return 0;   
}

PUBLIC HTParentAnchor *	HText_referenceAll ARGS1(HText *,me)
{
    return HTMainAnchor;
}

PUBLIC HTAnchor *	HText_linkSelTo ARGS2(HText *,me, HTAnchor *,anchor)
{
    return 0;
}


