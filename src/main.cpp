// main.cpp - the rudeserver program
//
// This example uses an xslt source file, and outputs the result to stdout.
// But, it uses an istream to hold the XML, instead of using an xml source file.
//
// Copyright (C) 2002, 2003, 2004, 2005 Matthew Flood
// See file AUTHORS for contact information
//
// This file is part of RudeServer.
//
// RudeServer is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// RudeServer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with RudeServer; (see COPYING) if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//------------------------------------------------------------------------




#include <rude/webplatform.h>
#include <rude/wordlet.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <string>
#include <cstdlib>



////// SABLOT ////////
//
#ifdef USING_SABLOT
#include <sablot.h>
#endif
//
//////////////////////


////// GNOME libxslt ////////
//
#ifdef USING_LIBXSLT
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif
//
//////////////////////


//////// XALAN ////////
//
#ifdef USING_XALAN_1_7
#include <xalanc/Include/PlatformDefinitions.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

#include <xalanc/ICUBridge/ICUBridge.hpp>
#include <xalanc/ICUBridge/FunctionICUFormatNumber.hpp>
#include <xalanc/ICUBridge/ICUXalanNumberFormatFactory.hpp>
#include <xalanc/ICUBridge/ICUBridgeCollationCompareFunctor.hpp>

XALAN_USING_XERCES(XMLPlatformUtils)
XALAN_USING_XALAN(XalanTransformer)
XALAN_USING_XALAN(XSLTInputSource)
XALAN_USING_XALAN(XSLTResultTarget)
XALAN_USING_XALAN(FunctionICUFormatNumber)
#endif
//
/////////////////////

using namespace std;
using namespace rude;

const char *getDeviceType(void);
void loadfile(const char *filepath, std::strstream &myxsl);
std::string device="";

int main(int argc, char *argv[])
{


	//////////////////////////////////////////////
	// LOAD THE CONFIGURATION FILE
	//
	Config config;
	{
		string configname = argv[0];
		configname += ".ini";

		Config::setDefaultConfigFile(configname.c_str());

		if(!config.load(configname.c_str()))
		{
			cerr << "Could not open main configuration file: " << configname << "\n";
			return 1;
		}
	}


	//////////////////////////////////////////
	// CONFIGURE THE DATABASE CONTEXTS
	// TODO: have a Database Context Section that names all context sections,
	// then have a section for each context with the relevant info
	//
	config.setSection("database contexts");
	int numcontexts=config.getNumDataMembers();
	for(int x=0; x< numcontexts; x++)
	{
          config.setSection("database contexts");
	  string contextname=config.getDataNameAt(x);
	  config.setSection(contextname.c_str());
	  Database::addContext(	contextname.c_str(),
							config.getStringValue("database"),
							config.getStringValue("username"), 
							config.getStringValue("password"),
							config.getStringValue("server"),
							config.getIntValue("port")
						);

	}


	///////////////////////////////////////////////
	// CONFIGURE THE SESSION OBJECT
	//
	config.setSection("sessions", true);
	Session::setPersistanceMethod("config");
	Session::setSessionDirectory(config.getStringValue("sessiondir"));
	Session::setPath(config.getStringValue("Session Path"));
	Session::setDomain(config.getStringValue("Session Domain"));
	Session::setCookieIdentifer(config.getStringValue("Cookie Identifier"));
	Session *session = Session::getSession();
	
	rude::CGI parser;	

	bool goodview=true;

	config.setSection("");



	////////////////////////////////////
	// EXECUTE THE REQUESTED COMMAND(s)
	// 			(If Any)
	// TODO: commands will build xml errors / messages
	// instead of a string
	//
	CommandFactory *cfactory = CommandFactory::instance();
		// Set the .so command libraries directory from the config object
		// if empty, the command factory should use default library locations
		//
		cfactory->setLibDir(config.getStringValue("CommandLib"));
	string commandmessage="";
	if(parser.exists("command") || parser.exists("action"))
	{
		string commandname = parser.value("command");
		if(commandname == "")
		{
			commandname = parser.value("action");
		}
		
		Command *command = cfactory->buildCommand(commandname.c_str());
		if(command)
		{
			if(!command->execute())
			{
				goodview=false;
			}
                        commandmessage = command->getMessage();
		}
	}

	
	string programurl=config.getStringValue("ProgramUrl");
	string secureprogramurl=config.getStringValue("SecureProgramUrl");
	
	ViewFactory *vfactory=ViewFactory::instance();
	
	// Set the .so view libraries directory from the config object
	//
	vfactory->setLibDir(config.getStringValue("ViewLib"));

	// Set the default view from the config object
	//
	vfactory->setDefaultLib(config.getStringValue("DefaultView"));


	std::string theDecorator = (parser["d"])[0] ? parser["d"] : config.getStringValue("Decorator");
	
	// Build the view object
	//
	View *myview;
	if(!goodview && parser.exists("errview"))
	{
		myview = vfactory->buildDecoratedView(parser.value("errview"),theDecorator.c_str());
	}
	else
	{
		myview = vfactory->buildDecoratedView(parser.value("view"),theDecorator.c_str());
	}

   myview->traceMessage(__FILE__, __LINE__, commandmessage.c_str());
	
	
	

	/////////////////////////////////////
	// 	OUTPUT THE VIEW
	//

	// Determine the Display Device, content-type of the device, 
	// and the accompanying list length (eg. wml=10, html=50, pdf=0 or all)
	// 
	
	std::string outputdevice=parser["d"];
	if(outputdevice == "")
	{
		outputdevice=getDeviceType();
	}
	if(parser.exists("print"))
	{
		outputdevice = "print";
	}
	string contenttype;
	if(outputdevice == "wml")
	{
		contenttype = "text/vnd.wap-wml";
	}
	else if(outputdevice == "plain")
	{
		contenttype = "text/plain";
	}
	else
	{
		contenttype = "text/html";
	}
		
	if(myview->requiresTranslation() && !parser.exists("forcexml") && outputdevice != "xml")
	{
		//
		// THIS BLOCK PERFORMS XSLT TRANSLATION
		//



		// Determine the display language and charset
		// this charset should be set in the 
		// appropriate Apache containers for this domain as well....
		//
		string charset=config.getStringValue("charset");
		if(charset == "")
		{
			charset = "ISO-8859-1";
		}



		//////////////////////////////////////////////////////////////////
		// DETERMINE LANGUAGE PATHS
		//
		//
      string languagebase = config.getStringValue("Language Base Path");

      if(languagebase == "")
		{
        languagebase="language/";
		}
		if(languagebase[ languagebase.size() -1 ] != '/')
		{
        languagebase += "/";
		}

      string language = (parser["lang"])[0] ? parser["lang"] :  config.getStringValue("Default Language");
      if(language == "")
		{
         language="default";
		}
		string languagepath=languagebase + language + "/";
		
      config.setSection("Language Encodings");

		string encoding=config.getStringValue(language.c_str());
		if(encoding == "")
		{
			encoding = "ISO-8859-1";
		}
		
		config.setSection("");




      
		
		string xslbase=config.getStringValue("xsl base");
		myview->setCharset(encoding.c_str());
		
		// Obtain the XML from the view object
		//
		std::ostrstream myout;		
		myview->display(myout);

		
		// Build the XSL StyleSheet
		//

		std::strstream myxsl;		
		myxsl << 
		"<?xml version='1.0' encoding='" << encoding << "'?>\n"
		"<!DOCTYPE xsl:stylesheet [\n"

		// We have wordlets now!!
		//"<!ENTITY nbsp '&#160;'>\n"
		"<!ENTITY nbsp '((NBSP))'>\n"

		"<!ENTITY copy '&#169;'>\n"
		"<!ENTITY quot '&#34;'>\n"
		"<!ENTITY program '" << argv[0] << "'>\n"
		"<!ENTITY programurl '" << programurl << "'>\n"
		"<!ENTITY secureprogramurl '" << secureprogramurl << "'>\n";


		// The config file has a section called DTD
		config.setSection("DTD");
		int num_el=config.getNumDataMembers();
		for(int y=0; y< num_el; y++)
		{
            string name=config.getDataNameAt(y);
			string value=config.getStringValue(name.c_str());
			myxsl << "<!ENTITY " << name << " '" << value << "'>\n";
		}

		config.setSection("");

      // Load common language dtd's
		//
      string basedtd = languagebase + "common.dtd";
		loadfile(basedtd.c_str(), myxsl);
		
      string dtd = languagepath + "common.dtd";
		loadfile(dtd.c_str(), myxsl);

		// Load all the language dtd's required by the views
		// if they are there
		//
		int num = vfactory->getNumXSLTemplates();
		for(int x=0; x< num; x ++)
		{
			string filepath=languagepath + vfactory->getXSLTemplateAt(x) + ".dtd";
		   loadfile(filepath.c_str(), myxsl);
		}

		myxsl <<
		"]>\n"
		"<xsl:stylesheet version='1.0' xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>\n";
		
		// Determine the xsl template path
		//
		string xsltemplates = xslbase + outputdevice + "/";
			
		// Load output.xsl - which contains additional top-level xsl directives, esp. xsl:output and xsl:decimal-formats
		// and could also contain Common templates
		//
		string outputxsl= xsltemplates + "output.xsl";
		loadfile(outputxsl.c_str(), myxsl);
			
		// Load all the templates required by the views
		// if they are there
		//
		num = vfactory->getNumXSLTemplates();
		for(int x=0; x< num; x ++)
		{
			string filepath=xsltemplates;
			filepath += vfactory->getXSLTemplateAt(x);
			filepath += ".xsl";
		   loadfile(filepath.c_str(), myxsl);
		}
		myxsl << "</xsl:stylesheet>\n";

	
		if(!parser.exists("forcexsl"))
		{
			// Output HTTP Headers
			//
			cout << "Content-Type: " << contenttype <<  "; charset=" << encoding << "\n\n";
			
			// perform XSLT
			//
         string myresult="";





#ifdef USING_SABLOT
			SablotSituation S;
			SablotHandle proc;

			SablotCreateSituation(&S);

			SablotCreateProcessorForSituation(S, &proc);

			SablotAddArgBuffer(S, proc, "sheet", myxsl.str());
			SablotAddArgBuffer(S, proc, "data", myout.str());

			SablotRunProcessorGen(S, proc, "arg:/sheet", "arg:/data", "arg:/out");

			char * result;
			SablotGetResultArg(proc, "arg:/out", &result);

			myresult = result;

			SablotFree(result);
			SablotDestroyProcessor(proc);
			SablotDestroySituation(S);
#endif

#ifdef USING_LIBXSLT

			// The document holders
			//
			xmlDocPtr xmldoc, stylesheet, res; // res = result
			xsltStylesheetPtr cur = NULL;
			
			// Initizlize engine
			//
			xmlSubstituteEntitiesDefault(1);
			xmlLoadExtDtdDefaultValue = 1;

			// Parse the XML input
			// function declared in gnome-xml/parser.h
			//
			xmldoc = xmlParseMemory (myout.str(),  myout.pcount());
			//  xmlDebugDumpDocument( stdout, xmldoc);
			
			// Parse the XSL input
			// function declared in gnome-xml/parser.h
			// function declared in libxslt/xsltInternals.h
			//
			stylesheet = xmlParseMemory (myxsl.str(),  myxsl.pcount());
			// xmlDebugDumpDocument( stdout, stylesheet);
			cur = xsltParseStylesheetDoc(stylesheet);


			// Apply the template
			//
			const char *params[1];
			params[0] = NULL;
			res = xsltApplyStylesheet(cur, xmldoc, params);

			// Get the results
			// function declared in libsxlt/xsltutils.h
			//
			// xsltSaveResultToFile(stdout, res, cur);
			//
			xmlChar * my_doc_txt_ptr;
			int my_outlength;

			xsltSaveResultToString(&my_doc_txt_ptr, &my_outlength, res, cur);

         myresult = (char *) my_doc_txt_ptr;

			// Clean up documents
			// 
			xsltFreeStylesheet(cur);
			xmlFreeDoc(res);
			xmlFreeDoc(xmldoc);
			
			// this gets cleaned up with res - which has a pointer to the doc
			// xmlFreeDoc(stylesheet);

			// Clean up engine
			//
			xsltCleanupGlobals();
			xmlCleanupParser();

#endif

#ifdef USING_XALAN_1_7
			XMLPlatformUtils::Initialize();
			XalanTransformer::initialize();
			{
				XalanTransformer theXalanTransformer;
				FunctionICUFormatNumber::FunctionICUFormatNumberInstaller theInstaller;
				
				istrstream iistr(myout.str(), myout.pcount());
				istrstream xslistr(myxsl.str(), myxsl.pcount());
				ostrstream myout;
				
				XSLTInputSource xmlIn2(&iistr);
				XSLTInputSource xslIn2(&xslistr);
				//XSLTResultTarget xmlOut2(cout);
				int theResult = theXalanTransformer.transform(xmlIn2, xslIn2, &myout);

            // terminate the ostrstream or you'll get buffer overflow output!!
            //
            myout << ends;
            myresult = myout.rdbuf()->str();

				if(theResult)
				{
					cout << "Error in transformation!!<br>\n";
					cout << "Result of Transformation  is " << theResult << " --\n";
					cout << theXalanTransformer.getLastError() << "--\n";
				}
			}
			XalanTransformer::terminate();
			XMLPlatformUtils::Terminate();
			XalanTransformer::ICUCleanUp();
#endif

                 // WORDLETS
                 //
                 if(!parser.exists("nowordlets"))
                 {
                      Wordlet wordlet;
                      config.setSection("WORDLET FILES");
                      int num=config.getNumDataMembers();
                      for(int x=0; x< num; x++)
                      {
                      		wordlet.addLanguageFile(config.getDataNameAt(x));
                      }
                      config.setSection("WORDLET ALT LANGUAGES");
                      num=config.getNumDataMembers();
                      for(int x=0; x< num; x++)
                      {
                      		wordlet.addLanguageAlternative(config.getDataNameAt(x));
                      }

                      config.setSection("WORDLETS");
                      wordlet.setLanguage(config.getStringValue("Language"));
                      if(parser.exists("WordletLanguage"))
                      {
                      		wordlet.setLanguage(parser["WordletLanguage"]);
                      }
                      if(parser.exists("showLocalUndefined"))
                      {
                      		wordlet.showLocalUndefined(true);
                      }
                      if(parser.exists("showUndefined"))
                      {
                      		wordlet.showUndefined(true);
                      }
                      wordlet.translate(myresult);
                 }

                 cout << myresult;

		}
		else
		{
			cout << "Content-Type: text/plain; charset=" << encoding << "\n\n";
			cout << myxsl.str();
		}
	}
	else
	{
		//
		// THIS BLOCK RETURNS THE RAW RESULT OF THE VIEW
		//
		
		myview->printHeaders();
		myview->display(std::cout);
	}

//	if(parser.exists("showlibs"))
//	{	
//		int num = vfactory->getNumXSLTemplates();
//		cout << "Loaded Libraries: <br>\n";
//		for(int x=0; x< num; x ++)
//		{
//			string filepath=xslbase;
//			filepath += templatedirectory;
//			filepath += vfactory->getXSLTemplateAt(x);
//			cout << filepath << " <br>\n";			
//		}
//	}

	///////////////////////////
	// PERFORM CLEAN-UP
	//

	// What we want to do, is if session isNew, then save it.
	// We can do that using the session's destructor
//	cout << "Performing Cleanup\n";
	if(session->isNew())
	{
		
		if(!session->save())
		{
	//		cout << "Error saving session : " << session->getError() << "\n";
		}
		else
		{
		//	cout << "Session Saved!";
		}
	}
	else
	{
		//cout << "Session is not new\n";
	}
	delete session;
		
	
	delete myview;
	delete vfactory;
	delete cfactory;
	return 0;
}

void loadfile(const char *filepath, std::strstream &myxsl)
{
	ifstream infile(filepath);
	if(infile)
	{
		myxsl << "<!-- Loaded from file: '" << filepath << "' -->\n";
			std::string line;
			while(!infile.eof())
			{	
				std::getline(infile, line);
				myxsl << line << "\n";
			}
			myxsl << "\n";
			infile.close();
		}
		else
		{
			myxsl << "<!-- Error opening template file: '" << filepath << "' -->\n";
		}
		myxsl << "\n";
}



const char *getDeviceType(void)
{
	
	std::string userAgent = getenv("HTTP_USER_AGENT") ? getenv("HTTP_USER_AGENT"): "";
	std::string acceptHeader= getenv("HTTP_ACCEPT") ? getenv("HTTP_ACCEPT") : "";

	int len=userAgent.size();
	for(int x=0; x< len; x++)
	{
		if(userAgent[x] >= 'a' && userAgent[x] <='z')
		{
			userAgent[x] = toupper(userAgent[x]);
		}
	}
	len=acceptHeader.size();
	for(int x=0; x< len; x++)
	{
		if(acceptHeader[x] >= 'a' && acceptHeader[x] <='z')
		{
			acceptHeader[x] = toupper(acceptHeader[x]);
		}
	}
	
	std::string::size_type pos;
	

	if(acceptHeader.find("VND.WAP.WML") != string::npos)
	{
		device="wml";
	}
	else
	if(userAgent.find("T610") != string::npos)
	{
		device="wml";

	}
	else
	if(userAgent.find("MICROSOFT URL CONTROL") != string::npos)
	{
		device="chtml";
	}
	else
	if(userAgent.find("PIXO") != string::npos)
	{
		device="chtml";
	}
	else
	{
		//System.out.println("detected browser device");
		device="html";
	}

	return device.c_str();

}
