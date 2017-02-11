
/**
 *
 * @file symbian.cpp
 *
 * Part of the OpenJazz project
 *
 * @section Licence
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include <eikenv.h>
#include <eikapp.h>
#include <eikappui.h>
#include <stdio.h>

#include <eikapp.h>
#include <e32base.h>
#include <sdlapp.h>
#include <BAUTILS.H>
#include <e32math.h>

char KOpenJazzPath[256];
FILE* mystdout = NULL;
FILE *mystderr = NULL;

/// Symbian app
class COpenJazzApp: public CSDLApp {
public:
	COpenJazzApp();
   	~COpenJazzApp();
#ifdef UIQ3
	/**
	 * Returns the resource id to be used to declare the views supported by this UIQ3 app
 	 * @return TInt, resource id
   	*/
	TInt ViewResourceId();
#endif
	/**
	 * This has a default empty implementation.
	 * Is called just before SDL_Main is called to allow init of system vars
	 */
	virtual	void PreInitializeAppL();

	TUid AppDllUid() const;

};


CApaApplication* NewApplication() {
	// Return pointer to newly created CQMApp
	return new COpenJazzApp;
}

#include <eikstart.h>
// E32Main() contains the program's start up code, the entry point for an EXE.
GLDEF_C TInt E32Main() {
 	return EikStart::RunApplication(NewApplication);
}


COpenJazzApp::COpenJazzApp() {
}

COpenJazzApp::~COpenJazzApp() {
	fflush(mystdout);
	fflush(mystderr);
	fflush(stdout);
	fflush(stderr);
	fclose(stdout);
	fclose(stderr);
}
#ifdef UIQ3
#include <OpenJazz.rsg>
/**
 * Returns the resource id to be used to declare the views supported by this UIQ3 app
 * @return TInt, resource id
 */
TInt COpenJazzApp::ViewResourceId() {
	return R_SDL_VIEW_UI_CONFIGURATIONS;
}
#endif

/**
 * This has a default empty implementation.
 * Is called just before SDL_Main is called to allow init of system vars
 */
void COpenJazzApp::PreInitializeAppL()
{
	TFileName filename;
	TPtr8 ptr((unsigned char*) KOpenJazzPath, 0, 255);
#ifdef UIQ3
	mystdout = fopen("c:\\shared\\openjazz\\stdout.txt","w+");
	mystderr = fopen("c:\\shared\\openjazz\\stderr.txt","w+");
#else
	mystdout = fopen("c:\\data\\openjazz\\stdout.txt","w+");
	mystderr = fopen("c:\\data\\openjazz\\stderr.txt","w+");
#endif
	*stderr = *mystdout;
	*stdout = *mystderr;
	filename = _L("C:\\openjazz\\");
	for(TInt i = 'D';i<'Z';i++)
	{
		filename[0] = i;
		if(BaflUtils::PathExists(CEikonEnv::Static()->FsSession(), filename))
		{
			ptr.Copy(filename);
			ptr.ZeroTerminate();
			return;
		}
	}

#ifdef UIQ3
	ptr.Copy(_L8("c:\\shared\\openjazz\\"));
#else
	ptr.Copy(_L8("c:\\data\\openjazz\\"));
#endif
	ptr.ZeroTerminate();
}

/**
 *   Responsible for returning the unique UID of this application
 * @return unique UID for this application in a TUid
 **/
TUid COpenJazzApp::AppDllUid() const {
	return TUid::Uid(0xA000A005);
}

float sinf(float value)
{
	TReal ret;
	Math::Sin(ret, value);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////



