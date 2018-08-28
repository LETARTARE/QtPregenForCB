/***************************************************************
 * Name:      qtpre.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Modified:  2018-05-02
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "qtpre.h"

#include <sdk.h>
#include <cbplugin.h>      // sdk version
#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
// for linux
#include <wx/dir.h>
#include <projectfile.h>
#include "infowindow.h"     // InfoWindow::Display(
// not place change !
#include "print.h"
///-----------------------------------------------------------------------------
///	Constructor
///
/// Called by :
///		1. qtPrebuild::qtPrebuild(cbProject * prj, int logindex, wxString & nameplugin):1,
///
qtPre::qtPre(cbProject * prj, int logindex, const wxString & nameplugin)
	: m_namePlugin(nameplugin), m_pProject(prj),
	  m_nameProject(_T("")),
	  m_Mexe(_T("")), m_Uexe(_T("")), m_Rexe(_T("")), m_Lexe(_T("")),
	  m_pm(Manager::Get()),
	  m_pMam(Manager::Get()->GetMacrosManager() ),
	  m_LogPageIndex(logindex)
	//  , Mes(_T("")),
{
	if (m_pProject)
	{
		m_nameProject = m_pProject->GetTitle();
		m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
		m_pTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
	}
	else
		m_nameProject = wxEmptyString;

#if   defined(__WXMSW__)
	m_Win = true; m_Linux = m_Mac = false;
#elif defined(__WXGTK__)
	m_Linux = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Linux = false;
#endif
	platForm();
	//
	m_Moc = _T("moc"); m_UI = _T("ui"); m_Qrc = _T("qrc"); m_Lm = _T("qm") ;
}
///-----------------------------------------------------------------------------
///	Destructor
///
/// Called by
///		1. qtPrebuild::~qtPrebuild():1,
///
qtPre::~qtPre()
{
}
///-----------------------------------------------------------------------------
/// Get date
///
/// Called by  :
///		1. beginMesBuildCreate():1,
///		2. endMesBuildCreate():1
///
wxString qtPre::date()
{
	wxDateTime now = wxDateTime::Now();
	//wxString str = now.FormatISODate() + _T("-") + now.FormatISOTime();
	wxString str = now.FormatDate() + _T("-") + now.FormatTime();

    return str ;
}
///-----------------------------------------------------------------------------
/// Execution time
///
/// Called by  :
///		1. endMesBuildCreate():1
///		2. endMesFileCreate():1,
///		3. endMesClean():1,
///
wxString qtPre::duration()
{
    clock_t dure = clock() - m_start;
    if (m_Linux)
        dure /= 1000;

    return wxString::Format(_T("%ld ms"), dure);
}
///-----------------------------------------------------------------------------
///  Begin duration for Debug
///
void qtPre::beginDuration(const wxString & namefunction)
{
// date
	Mes = _T("start of ") + namefunction  + _T(" : ") + date()  ;
	printErr(Mes);
	m_start = clock();
}

///-----------------------------------------------------------------------------
///  End duration for Debug
///
void qtPre::endDuration(const wxString & namefunction)
{
	Mes = _T("end of ") + namefunction  + _T(" : ") + date() + _T(" -> ") + duration();
	printErr(Mes);
}
///-----------------------------------------------------------------------------
/// Give internal plugin name
///
/// Called by :
///		1. QtPregen::OnPrebuild(CodeBlocksEvent& event):1,
///
wxString qtPre::namePlugin()
{
	return m_namePlugin;
}
///-----------------------------------------------------------------------------
/// Determines the system platform
///
/// Called by :
///		1. qtPre(cbProject * prj, int logindex, wxString & nameplugin):1,
///
void qtPre::platForm()
{
// choice platform
	if (! m_Win)
	{
		#undef SepD
		#define SepD 13
		#undef Sizesep
		#define Sizesep 1
	// tables  ( >= Qt-5.9 )
       //   TODO ????
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt4
		m_TablibQt.Add(_T("qtgui4"),1) ;
		m_TablibQt.Add(_T("qtcore4"),1) ;
		m_TablibQt.Add(_T("qtguid4"),1) ;
		m_TablibQt.Add(_T("qtcored4"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
	}

	if (m_Win)
	{
		#undef Eol
		#define Eol CrLf
	// tables
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt4
		m_TablibQt.Add(_T("qtgui4"),1) ;
		m_TablibQt.Add(_T("qtcore4"),1) ;
		m_TablibQt.Add(_T("qtguid4"),1) ;
		m_TablibQt.Add(_T("qtcored4"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
			// <= qt5.9
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
	}
	else
	if (m_Mac)
	{
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux)
	{
		#undef Eol
		#define Eol Lf
	}
}
///-----------------------------------------------------------------------------
///  Get version SDK
///
///  Called by :
/// 	1. QtPregen::OnAttach():1,
///
wxString qtPre::GetVersionSDK()
{
	// from 'cbplugin.h'
	uint16_t major 	= PLUGIN_SDK_VERSION_MAJOR
			,minor 	= PLUGIN_SDK_VERSION_MINOR
			,release= PLUGIN_SDK_VERSION_RELEASE;
	Mes = (wxString() << major) + _T(".") + (wxString() << minor) + _T(".") + (wxString() << release);

    return  Mes ;
}

///-----------------------------------------------------------------------------
/// test clean
///
///  Called by :
///		none

bool qtPre::isClean()
{
	if (m_clean)
	{
		m_Filecreator.Clear();
		m_Registered.Clear();
		m_Createdfile.Clear();
		m_Filestocreate.Clear();
		m_Filewascreated.Clear();
	}

	return m_clean;
}

///-----------------------------------------------------------------------------
/// Set abort
///
/// Called by :
///		1. QtPregen::OnAbortAdding(CodeBlocksEvent& event):1,
///
void qtPre::Abort()
{
	m_abort = true;
}

///-----------------------------------------------------------------------------
///	Give the full complement filename  : target + "\" + filename
///
///  Called by :
///		1. complementDirectory():1,

wxString qtPre::fullFilename(const wxString & filename)
{
	wxString file = filename;
	wxString nameactivetarget = m_pProject->GetActiveBuildTarget() ;
	file = nameactivetarget + wxString(Slash) + file;

	return file;
}

///-----------------------------------------------------------------------------
///	Give the complement directory
///
///  Called by :
///		1. qtPrebuild::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		2. qtPrebuild::addAllFiles():1,

wxString qtPre::complementDirectory() const
{
	wxString nameactivetarget = m_pProject->GetActiveBuildTarget() ;
	wxString dircomplement = m_dirPreBuild + nameactivetarget + wxString(Slash) ;

	return dircomplement;
}

///-----------------------------------------------------------------------------
///  Verify supplement file
///		ex : 'moc_xxx.cpp', 'ui_xxx.h', 'qrc_xxx.cpp',
///  	file format  = "qtprebuild\..._xxxx.ext",
///  	... = ["moc", "ui", "qrc"]
///
///  Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& event)
///
bool qtPre::isComplementFile(const wxString & file)
{
//Mes = _T("isComplementFile -> ") + file ;
//printWarn(Mes);
//print(allStrTable(_T("m_Registerd"), m_Registered));

// target name
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
	wxString filename = m_dirPreBuild + fullFilename(file) ;
//Mes = filename;
//printWarn(Mes);
	int16_t  index = m_Registered.Index (filename);
	bool ok = index != wxNOT_FOUND;

	return ok;
}

///-----------------------------------------------------------------------------
///  Search creator file  ex : 'xxx.h'
///		format 'm_Filecreator' ->  'dircreator\xxx.h'
///
///  Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& event)
///
bool qtPre::isCreatorFile(const wxString & file)
{
//Mes = _T("isCreatorFile -> ") + file ;
//printWarn(Mes);
//print(allStrTable(_T("m_Filecreator"), m_Filecreator));
	wxString dircreator = m_Filecreator.Item(0).BeforeLast(Slash)  ;
	wxString filename =  dircreator + Slash + file;

	int16_t  index = m_Filecreator.Index (filename);
	bool ok = index != wxNOT_FOUND;
//Mes = _T(" -> file = ") + file;
//Mes += _T(", filename = ") + filename;
//Mes += _T(", index = ") + (wxString() << index);
//print(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Gives the name of the file to create on
///		file = xxx.h(pp) 	->	moc_xxx.cpp
///		file = xxx.ui	->	ui_xxx.h
///		file = xxx.rc	->  rc_xxx.cpp
///		file = xxx.cpp	->  xxx.moc
///
/// Called by  :
///		1. qtPrebuild::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		2. qtPrebuild::unregisterFileComplement(const wxString & file, bool creator, bool first):1,
///		3. addAllFiles():1,
///
wxString qtPre::nameCreated(const wxString& file)
{
	wxString name  = file.BeforeLast('.') ;
	wxString fout ;
	if (name.Find(Slash) > 0)
	{
	// short name
		name = name.AfterLast(Slash) ;
	// before path
		fout += file.BeforeLast(Slash) + wxString(Slash) ;
	}
	wxString ext  = file.AfterLast('.')  ;
//1- file *.ui  (forms)
	if ( ext.Matches(m_UI) )
		fout += m_UI + _T("_") + name + DOT_EXT_H  ;
	else
//2- file *.qrc  (resource)
	if (ext.Matches(m_Qrc) )
		fout += m_Qrc + _T("_") + name + DOT_EXT_CPP  ;
	else
//3- file *.h or *hpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP)  )
		fout +=  m_Moc + _T("_") + name + DOT_EXT_CPP ;
	else
//4- file *.cpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_CPP) )
		fout +=  name + DOT_EXT_MOC ;

	fout = fout.AfterLast(Slash) ;

	return fout  ;
}

///-----------------------------------------------------------------------------
///  Set index page to log
///
///  Called by :
/// 		none
///
void qtPre::SetPageIndex(int logindex)
{
	m_LogPageIndex = logindex;
}

///-----------------------------------------------------------------------------
/// Search virtual target
///
/// Called by  :
///		1. listRealsTargets(const wxString& nametarget):1,
///		2. detectQtProject(cbProject * prj, bool report):1,
///		3. detectQtTarget(const wxString& nametarget, cbProject * prj)
///		4. qtPrebuild::findGoodfilesTarget():1,
///		5. qtPrebuild::findGoodfiles():1,
///
bool qtPre::isVirtualTarget(const wxString& nametarget, bool warning)
{
// search virtual target : vtargetsTable <- table::wxArrayString
	wxArrayString vtargetsTable = m_pProject->GetVirtualBuildTargets () ;
	bool ok = false  ;
//	find 'nametarget' is virtual ?
	for (wxString namevirtualtarget : vtargetsTable)
	{
		ok = nametarget.Matches(namevirtualtarget) ;
		if (!ok)
			continue;

		if (warning)			{
			Mes = _T("'") + namevirtualtarget + _T("'") ;
			Mes += _(" is a virtual target !!") ;
			Mes += Lf + _("NOT YET IMPLEMENTED...") ;
			Mes += Lf + _("... you must use the real targets.")  ;
			//Mes += Lf + _("Cannot continue.") ;
			printErr(Mes);
			cbMessageBox(Mes, m_namePlugin + Space + _("plugin"), wxICON_ERROR)  ;
		}
		break;
	}

	return ok  ;
}

///-----------------------------------------------------------------------------
/// Search all not virtual target
///
/// Called by  :
///		1. detectQtProject(cbProject * prj, bool report):1,
///		2. qtPrebuild::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///
///	Calls to :
///		1. isVirtualTarget(const wxString& nametarget, bool warning):1,
///
wxArrayString qtPre::listRealsTargets(const wxString& nametarget)
{
	wxArrayString realsTargets;
	bool ok = isVirtualTarget(nametarget);
	if(ok)
	{
	// list virtual targets
		wxArrayString vtargetsTable = m_pProject->GetVirtualBuildTargets () ;
		int nvirtual = vtargetsTable.GetCount() ;
		ok = nvirtual > 0 ;
		if (ok)
		{
			ok = false  ;
		// virtual name
			for (wxString namevirtualtarget : vtargetsTable)
			{
				ok = nametarget.Matches(namevirtualtarget) ;
				if (!ok)
					continue;
            // all targets no virtuals
				//realsTargets = m_pProject->GetVirtualBuildTargetGroup(nametarget) ;
				realsTargets = m_pProject->GetExpandedVirtualBuildTargetGroup(nametarget);
				break;
			}
		}
	}
	else
	{
	    realsTargets.Add(nametarget);
    }

	return realsTargets;
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' Project : it contains at least one target Qt
///
/// Called by  :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& event):1,
///		2. QtPregen::OnNewProject(CodeBlocksEvent& event):1,
///		3. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& event):1,
///		4. QtPregen::OnAddComplements(CodeBlocksEvent& event):1,
///
/// Calls to :
///		1. listRealsTargetsconst wxString& nametarget):1,
///		2. hasLibQtCompileTargetBase * container):2,
///		3. findLibQtToTargets():2,
///
bool qtPre::detectQtProject(cbProject * prj, bool report)
{
//Mes = _T("=> 'qtPre::detectQtProject(...)'") ;
//printWarn(Mes);
	if (! prj)
		return false;

// project name
	m_pProject = prj;
	m_nameProject = m_pProject->GetTitle() ;
//Mes = _T("m_nameProject = ") + m_nameProject;
//printWarn(Mes);
    m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
// attention : might be virtual target !
	//if (isVirtualTarget(m_nameActiveTarget, false))
	if (isVirtualTarget(m_nameActiveTarget))
    // peek the first real target
		m_nameActiveTarget = listRealsTargets(m_nameActiveTarget).Item(0);
//Mes = _T("first real target = ") + m_nameActiveTarget;
//printWarn(Mes);
    ProjectBuildTarget* buildtarget =  m_pProject->GetBuildTarget(m_nameActiveTarget) ;
    if (!buildtarget)
		return false;
//Mes = _T("builtarget-> Title = ") + buildtarget->GetFullTitle();
//printWarn(Mes);
// option relation target <-> project for compiler
    OptionsRelation rel = buildtarget->GetOptionRelation(ortCompilerOptions);
//Mes = _T("rel = ") + (wxString() << (int)rel);
//printWarn(Mes);
/** rel =
 *   orUseParentOptionsOnly = 0,
 *   orUseTargetOptionsOnly = 1,   : independent target
 *   orPrependToParentOptions = 2,
 *   orAppendToParentOptions = 3,
 */
    bool ok03 = false, ok12 = false;
// use parent only or append target to parent ?
    // 0 or 3
    if (rel == orUseParentOptionsOnly || rel == orAppendToParentOptions)
    {
        // search libraries to project
        ok03 = hasLibQt(m_pProject) ;
        if (!ok03 && rel == orAppendToParentOptions)
         // search libraries to all targets
            ok03 = findLibQtToTargets();
    }
//Mes = _T("ok03 = ") + (wxString() << (int)ok03);
//printWarn(Mes);

// use target only or prepend target to parent ?
    // 1 or 2
    if (rel == orUseTargetOptionsOnly || rel == orPrependToParentOptions)
    {
         // search libraries to targets
        ok12 = findLibQtToTargets();
//Mes = _T("ok12 = ") + (wxString() << (int)ok12);
//printWarn(Mes);
        if (!ok12 && rel == orPrependToParentOptions)
        // search libraries to project
            ok12 = hasLibQt(m_pProject) ;
    }
//Mes = _T("ok12 = ") + (wxString() << (int)ok12);
//printWarn(Mes);
	bool valid = ok03 || ok12;
	if (valid)
	{
		if (report)
		{
			wxString  title = _("The project") + Space + m_nameProject + Space + _("uses Qt libraries !") ;
			Mes = _("It will generate the complements files...") ;
			InfoWindow::Display(title, Mes, 5000);
		}

		bool usemake = m_pProject->IsMakefileCustom() ;
		if(usemake)
		{
			Mes = _T("... but please, DISABLE using of custom makefile");
			Mes += Lf + Quote + m_namePlugin + Quote + _T(" not use makefile.");
			print(Mes);

			Mes += Lf + _("CAN NOT CONTINUE !") ;
			cbMessageBox(Mes, _("Used makefile !!"), wxICON_WARNING ) ;
			valid = false;
			Mes = m_namePlugin + _T(" -> ") + _("end") + _T(" ...");
			printWarn(Mes);
			return valid ;
		}
	}

	return valid;
}
///-----------------------------------------------------------------------------
/// Detection of a 'Qt' Target
///
/// Called by  :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& event):1,
///		2. QtPregen::OnNewProject(CodeBlocksEvent& event):1,
///		3. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& event):1,
///		4. QtPregen::OnActivateTarget(CodeBlocksEvent& event):1,
///
/// Calls to :
///		1. isVirtualTarget(const wxString& nametarget, bool warning):1,
///		2. hasLibQtCompileTargetBase * container):2,
///
bool qtPre::detectQtTarget(const wxString& nametarget, cbProject * prj)
{
	if(!prj)
		return false;

	bool ok = false;
// is virtual target ?
	if (isVirtualTarget(nametarget))  // withless advice !
		return ok;
//Mes =  Tab + Quote + nametarget + Quote + _T(" is NOT virtual") ;
//printWarn(Mes);

	m_pProject = prj;
// is a real target
	ProjectBuildTarget *buildtarget = m_pProject->GetBuildTarget(nametarget);
	if (! buildtarget)
		return ok;
//Mes =  Tab + Quote + nametarget + Quote + _T(" exists") ;
//printWarn(Mes);

	ok = hasLibQt(buildtarget);
	if (!ok)
	{
		// search into project libraries only, not others targets

	}
//Mes = _T("detectQtTarget(...) = ") + (wxString() << ok);
//printWarn(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Return true if find library QT in targets of 'm_pProject'
///
/// Called by  :
///		1. detectQtProject(cbProject * prj, bool report = false):2,
///
///	Calls to :
///		1. hasLibQt(CompileTargetBase * container):1,
///
bool qtPre::findLibQtToTargets()
{
    bool ok = false ;
// search library to targets
    ProjectBuildTarget* buildtarget;
    uint16_t nt = 0 , ntargets = m_pProject->GetBuildTargetsCount() ;
    while (nt < ntargets && ! ok )
    {
    // retrieve the target libraries  'ProjectBuildTarget* builstarget'
        buildtarget = m_pProject->GetBuildTarget(nt++) ;
        if (!buildtarget)
            continue ;
//Mes = (wxString() << nt) + _T(" -> ") + Quote + buildtarget->GetTitle() + Quote;
//printWarn(Mes);
        ok = hasLibQt(buildtarget) ;
        if (ok)
            break;
    }
//Mes = _T("findLibQtToTargets() = ") + (wxString() << ok);
//printWarn(Mes);

    return ok;
}

///-----------------------------------------------------------------------------
/// Return true if good  'CompileTargetBase* container'
///
/// Called by  :
///		1. detectQtProject(cbProject * prj, bool report = false):2,
///		2. findLibQtToTargets():1,
///
bool qtPre::hasLibQt(CompileTargetBase * container)
{
//Mes = _T("into 'qtPre::hasLibQt(...)'") ;
//printWarn(Mes);
	bool ok = false;
	if (!container)
		return ok;

	wxArrayString tablibs = container->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
//Mes = _T("nlib = ") + (wxString() << nlib);
//printWarn(Mes);
	if (nlib > 0 )
	{
		wxString namelib ;
		uint16_t u=0;
		int16_t index= -1, pos ;
		while (u < nlib && !ok )
		{
			// lower, no extension
			namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
//Mes = (wxString() << u) + _T(" -> ") + Quote + namelib + Quote;
//printWarn(Mes);
			// no prefix "lib"
			pos = namelib.Find(_T("lib")) ;
			if (pos == 0)
				namelib.Remove(0, 3) ;
			// begin == "qt"
			pos = namelib.Find(_T("qt")) ;
//Mes = _T("pos 'qt' = ") + (wxString() << pos);
//printWarn(Mes);
			if (pos != 0)
				continue ;
			// find
/// ATTENTION : the table should then contain all Qt libraries !!
			index = m_TablibQt.Index(namelib);
//Mes = _T("index = ") + (wxString() << index);
//printWarn(Mes);
			ok = index != -1 ;
			// first finded
			if (ok)
				break;
		}
	}
//Mes = _T("haslibQt(...) = ") + (wxString() << ok);
//printWarn(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
///	Check if the target is independent
///
///  Called by :
///		1. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& event):1,

bool qtPre::isIndependentTarget(cbProject * prj, const wxString & target)
{
	// search the complements directory if there is a subdirectory
	// with the same name as the target
	m_dirProject = prj->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
//Mes = _T("dirproject = ") + Quote + m_dirProject + Quote;
//printWarn(Mes);
	wxDir dir(m_dirPreBuild );
	bool ok = ::wxDirExists(m_dirPreBuild );
	if (ok)
	{
		ok = dir.HasSubDirs(target);
	}
	else
	{
		Mes = _("The complements directory is not exists !") ;
		printWarn(Mes);
	}

	return ok;
}

///-----------------------------------------------------------------------------
/// Create a new directory,transfer all files to the new directory, delete old
///	directory
///
/// calle by :
///		1. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& event):1,

bool qtPre::newNameComplementDirTarget(wxString & newname, wxString & oldname)
{
	m_dirProject = m_pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
//Mes = _T("dirproject = ") + Quote + m_dirProject + Quote;
//printWarn(Mes);
	bool ok = wxFileName::DirExists(m_dirPreBuild);
	if (ok)
	{
        wxFileName filesystem;
		filesystem.AssignDir(m_dirPreBuild);
	// create a new directory
		ok = filesystem.Mkdir(newname);


	}
	return ok;
}

///-----------------------------------------------------------------------------
/// Detects already existing complement files in the project and
///	populates complement tables with existing files
///
/// Called by :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& event):1,
///		2. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& event):2,
///		3. QtPregen::OnAddComplements(CodeBlocksEvent& event):1,
///
/// Calls to :
///		1. refreshTables():1,

bool qtPre::detectComplementsOnDisk(cbProject * prj, const wxString & nametarget,  bool report)
{
/// DEBUG
//* **********************************
//	beginDuration(_T("qtPre::detectComplementsOnDisk(...)"));
//* *********************************
	if (!nametarget.IsEmpty())
		m_nameActiveTarget = nametarget;
	m_dirProject = prj->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
	wxString diradding = m_dirPreBuild + m_nameActiveTarget ;
//Mes = _T("diradding  = ") + Quote + diradding  + Quote;
//printWarn(Mes);
	bool ok = wxDirExists(diradding);
	if (ok)
	{
		m_Filewascreated.Clear();
	// register all complement files in 'm_Filewascreated'
		size_t n = wxDir::GetAllFiles(m_dirPreBuild, &m_Filewascreated );
	// complement files in target
		wxArrayString  filewascreated;
		n = wxDir::GetAllFiles(diradding, &filewascreated );
		Mes = Tab + _(", with") + Space;
		if (report)
		{
			if (n)
			{
				Mes += (wxString() << n) ;
				Mes += Space + _("complement files already created on disk in") ;
				Mes += Space + Quote + diradding + Quote;
			}
			else
			{
				Mes += _("no complement file to disk.") ;
			}
			print(Mes);
		}
		Mes = wxEmptyString;
		ok = ! m_Filewascreated.IsEmpty();
	// rebuild all tables
		if (ok)
		{
			refreshTables();
		}
	// error complements ?
		else
		{
			// advice ?
		//	Mes =  _("Complements detected error !!");
		//	printErr(Mes) ;
		}
	}

/// DEBUG
//* **********************************
//	endDuration(_T("qtPre::detectComplementsOnDisk(...)"));
//* *********************************

	return ok;
}

///-----------------------------------------------------------------------------
///	Refresh all the tables
///
///	Called by :
///		-# detectComplementsOnDisk(cbProject * prj, const wxString & nametarget,  bool report):1,
///	Calls to :
///		1. copyArray (const wxArrayString& strarray,  int nlcopy = 0):2,
///		2. wasCreatedToCreator(const wxArrayString& strarray):1,
///
bool qtPre::refreshTables(bool debug)
{
//Mes = _T("qtPre::refreshTables(...)");
//print(Mes);
/// DEBUG
//* **********************************
//	beginDuration(_T("qtPre::refreshTables(...)"));
//* *********************************
	bool ok = true;

	m_Registered.Clear();
	m_Registered = copyArray(m_Filewascreated ) ;
///
	m_Createdfile.Clear();
	m_Createdfile = copyArray(m_Filewascreated ) ;
///
	m_Filecreator.Clear();
	m_Filecreator = wasCreatedToCreator() ;
///
	m_Filestocreate.Clear();
	for (wxString item :  m_Filewascreated)
	{
		m_Filestocreate.Add(m_Devoid);
	}
	if (debug)
	{
		// DEBUG
		Mes = allStrTable(_T("m_Filewascreated"), m_Filewascreated);
		print(Mes);

		Mes = allStrTable(_T("m_Registered"), m_Registered);
		print(Mes);

		Mes = allStrTable(_T("m_Createdfile"), m_Createdfile);
		print(Mes);

		Mes = allStrTable(_T("m_Filecreator"), m_Filecreator);
		print(Mes);

		Mes = allStrTable(_T("m_Filestocreate"), m_Filestocreate);
		print(Mes);
	}

/// DEBUG
//* **********************************
//	endDuration(_T("qtPre::refreshTables(...)"));
//* *********************************
	return ok;
}

///-----------------------------------------------------------------------------
/// Copy a 'wxArrayString' to an another
///
/// Called by  :
///		-# qtPrebuild::filesTocreate(bool allrebuild):1,
///		-# qtPrebuild::createFiles():4,
///		-# refreshTables(bool debug):2,
///
wxArrayString qtPre::copyArray (const wxArrayString& strarray, int nlcopy)
{
	wxArrayString tmp ;
	int nl = strarray.GetCount()  ;
	if (!nl)
		return  tmp ;
    // adjust number of lines to copy
	if (nlcopy > 0 && nlcopy <= nl)
		nl = nlcopy;
	// a line
	wxString line;
	for (int u = 0; u < nl; u++)
	{
	// read strarray line
		line = strarray.Item(u) ;
	// write line to tmp
		tmp.Add(line, 1) ;
	}

	return tmp ;
}

///-----------------------------------------------------------------------------
/// Give a string from a 'wxArrayString' for debug
///
/// Called by  :  for debug
///
wxString qtPre::allStrTable(const wxString& title, const wxArrayString& strarray)
{
	wxString mes = _T("--------- debug --------------");
	mes += Lf + Quote + m_dirProject + Quote ;
	mes += Lf +_T("=> ") + Quote + title + Quote ;
	uint16_t nl = strarray.GetCount();
	mes += _T(" : ") + (wxString()<<nl) + Space + _("files") ;
	for (uint16_t u = 0; u < nl; u++)
	{
	// read strarray line from  '1' ... 'nl'
		mes += Lf + (wxString()<<u+1) + _T("- ") + strarray.Item(u) ;
	}
	mes +=  Lf + _T("---------end  debug ------------") ;

	return mes;
}

///-----------------------------------------------------------------------------
/// For create 'm_FilesCreator' from 'm_Filewascreated'
/// reverse a 'wxArrayString' to an another
///
/// Called by  :
///		1. refreshTables():1,
///	Calls to :
///		1. toFileCreator(const wxString& fcreated):n,
///
wxArrayString qtPre::wasCreatedToCreator()
{
// read file list to 'm_Filewascreated' with (moc_....cpp, ui_....h, qrc_....cpp, __Devoid__)
// write file list to 'tmp' with  (*.h, *.cpp, *.qrc, *.ui)
	wxArrayString tmp;
	wxString fullnameCreator;
	for (wxString fcreated: m_Filewascreated)
	{
		fullnameCreator = toFileCreator(fcreated.AfterLast(Slash))  ;
	//add to array
		tmp.Add(fullnameCreator);
	}

	return tmp;
}

///-----------------------------------------------------------------------------
/// For create  'creator files' from  'created files'
/// reverse a (string' to an another
///
/// Called by  :
///		1. wasCreatedToCreator():1,
///	Calls to :
///		1. fullFileCreator(const wxString&  fcreator):1,
///
wxString qtPre::toFileCreator(const wxString& fcreated)
{
	wxString fcreator , prepend, ext, name, fullname;
//Mes = _T(" fcreated = ") + Quote + fcreated + Quote ;
//printWarn(Mes);
	prepend = fcreated.BeforeFirst('_') ;
	name = fcreated.AfterFirst('_') ;
	name = name.BeforeFirst('.');
	ext = fcreated.AfterLast('.');
//1- file 'ui_uuu.h'  (forms)  -> 'uuu.ui'
	if ( prepend.Matches(m_UI) && ext.Matches(EXT_H))
		fcreator =  name + DOT_EXT_UI ;
	else
//2- file 'qrc_rrr.cpp'  (resource) -> 'rrr.qrc'
	if (prepend.Matches(m_Qrc) )
		fcreator = name + DOT_EXT_QRC  ;
	else
//4- file 'moc_mmm.cpp'  -> 'mmm.h'
	if (prepend.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_H ;
//5- file 'xxx.moc'  -> 'xxx.cpp'
	if (ext.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_CPP ;
//6- full name
	fullname = fullFileCreator(fcreator);
//Mes = _T(" fcreator = ") + Quote + fcreator + Quote ;
//Mes += _T(" ->  fullname = ") + Quote + fullname + Quote ;
//printWarn(Mes);

	return fullname;
}

///-----------------------------------------------------------------------------
/// Gives a relative to common top level path of 'fcreator';
///		example 'parserfile.h' => 'src/parser/parserfile.h'
///
/// Called by  :
///		1. toFileCreator(const wxString& fcreated):1,
///

wxString qtPre::fullFileCreator(const wxString&  fcreator)
{
	wxString fullname = wxEmptyString ;
	if (!m_pProject || !m_pTarget)
		return fullname;
//Mes = Tab + _T(" ==> search fcreator = ") + Quote + fcreator + Quote ;
// all files project
	uint16_t nfproject = m_pProject->GetFilesCount();
	if (!nfproject)
		return fullname;

	ProjectFile * prjfile ;
	ProjectBuildTarget* target;
	wxArrayString tabtargets ;
	bool good = false;
	for (uint16_t  nf = 0; nf < nfproject ; nf++)
	{
		prjfile = m_pProject->GetFile(nf);
        if (!prjfile)
			continue;
        // all real build targets
		tabtargets = prjfile->GetBuildTargets() ;
		for ( wxString nametarget : tabtargets)
		{
			if(nametarget.Matches(m_nameActiveTarget) )
			{
				target = m_pProject->GetBuildTarget(nametarget);
				if (target->GetTargetType() == ttCommandsOnly)
				   continue;
//wxString mes = _("nametarget = ") + Quote + nametarget + Quote;
				fullname  = prjfile->relativeToCommonTopLevelPath ;
				if (fullname.IsEmpty() || fullname.StartsWith(m_dirPreBuild) )
					continue;
//mes += _(" => fullname = ") + Quote + fullname+ Quote;
//printWarn(mes);
				good = fullname.Contains(fcreator);
				if (good)
					break;
			}
		}
		if (good)
			break;
	}
	if (!good)
	{
		Mes = _T("The") + Space + Quote + fcreator + Quote ;
		Mes += Space + _T("file is missing in the CB project !");
		printErr(Mes);
		fullname = wxEmptyString ;
	}

	return fullname;
}

/*
// CRASH C//B !
wxString qtPre::fullFileCreator(const wxString&  fcreator)
{
Mes = _T(" fcreator = ") + Quote + fcreator + Quote ;
printWarn(Mes);
	wxString fullname = wxEmptyString ;
	if (!m_pProject || !m_pTarget)
		return fullname;

	uint16_t nftarget = m_pTarget->GetFilesCount();
	ProjectFile * prjfile;
	wxFileName filename ;
	for (uint16_t  nf = 0; nf < nftarget ; nf++)
	{
		prjfile = m_pTarget->GetFile(nf);
        if (!prjfile)
			continue;
		//fullname  = prjfile->GetRelativeFilename();
		//fullname  = prjfile->relativeToCommonTopLevelPath ;
		filename = prjfile->file ;
		fullname = filename.GetLongPath();
Mes = _("fullname = ") + Quote + fullname + Quote;
printWarn(Mes);
		if (fullname.Contains(fcreator))
			break;
	}

    // gives all project files
	FilesList listFiles = m_pProject->GetFilesList();

    bool ok = false;
	for (ProjectFile *prjfile : listFiles)
	{
		//fullname = prjfile->relativeToCommonTopLevelPath;
		fullname = prjfile->relativeFilename ;
Mes = _("fullname = ") + Quote + fullname + Quote;
print(Mes);
		ok = fullname.Contains(fcreator);
		if(ok)
			break;
	}

	return fullname;
}
*/

///-----------------------------------------------------------------------------
