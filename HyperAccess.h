//	A HyperAccess object provides access to hyperinformation, using particular
//	protocols and data format transformations.

// History:
//	26 Sep 90	Written TBL

#import <objc/Object.h>
#import <objc/List.h>
#import "Anchor.h"
#import "HyperText.h"

@interface HyperAccess:Object

//	Target variables for interface builder hookups:

{
    id  openString;
    id	keywords;
    id	contentSearch;
    
}

//	Interface builder initialisation methods:

- setOpenString:anObject;
- setKeywords:anObject;
- setContentSearch:anObject;

//	Action methods for buttons etc:

- search:sender;
- searchRTF: sender;
- searchSGML: sender;

- open: sender;
- openRTF:sender;
- openSGML:sender;
- saveNode:(HyperText *)aText;

//	Calls form other code:

- loadAnchor:(Anchor *)a;			// Loads an anchor.
- loadAnchor:(Anchor *)a Diagnostic:(int)level ;// Loads an anchor.

//	Text delegate methods:

- textDidChange:textObject;
- (BOOL)textWillChange:textObject;

//	HyperText delegate methods:

- hyperTextDidBecomeMain:sender;


@end
