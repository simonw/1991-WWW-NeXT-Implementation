//	HyperText Access method manager Object			HyperManager.h
//	--------------------------------------
//
//
// History:
//	   Oct 90	Written TBL
//	   Jan 92	Changed to include new W3 common code
//
#import "HyperAccess.h"
#import <objc/List.h>

@interface HyperManager : HyperAccess

{
	List * accesses;
}

- traceOn:sender;		// 	Diagnostics: Enable output to console
- traceOff:sender;		//	Disable output to console
- back:sender;			// 	Return whence we came
- next:sender;			//	Take link after link taken to get here
- previous:sender;		//	Take link before link taken to get here
- goHome:sender;		//	Load the home node
- goToBlank:sender;		//	Load the blank page
- help:sender;			//	Go to help page
- closeOthers:sender;		//	Close unedited windows
- save: sender;			//	Save main window's document
- saveAll:sender;		//	Save back all modified windows
- print:sender;			//	Print the main window
- runPagelayout:sender;		//	Run the page layout panel for the app.

- windowDidBecomeKey:sender;	//	Window delegate method

@end
