#include "module.h"
#define VER "1.0.1"
/**
************************************************************************************************
** Module : os_spamfilter.cpp
** Version : 1.0.0
** Author : skutte (SkyChatz1234@gmail.com)
** Release : 5th May, 2023
** Update : -
** Github : https://github.com/SkyChatz/anope-modules
************************************************************************************************
** <Description>
**
** This module is base on spamffiler.c by JeffK https://modules.anope.org/index.php?page=view&id=262
** We rewrite make it running on Anope V2
**
** It is recommended to use latest Anope version
**
** </Description>
**
************************************************************************************************
** Tested: 2.0.12, Unrealircd 6+
**
** Providing Command:
**
** /msg OperServ HELP spamfilter
** /msg OperServ spamfilter [Match Type] [Target] [Acttion] [Match] 
** or
** fantasy
** !spamfilter Simple Channel Kill *kill all the slave*
**
** Contact me
** Email : SkyChatz1234@hotmail.com
** IRC : /s irc.skychatz.org:7000 #modules
**
** You may use this module under the terms of docs/LICENSE, in the Anope source directory
**
** Configuration to put into your operserv config:
** module { name = "os_spamfilter"; }
** spamfilter {
**		config_path = "home/your/ircd/path/spamfilter.conf"
**		reason = "No Spam/Advertising On Our Network"
**}
** command { service = "OperServ"; name = "SPAMFILTER"; command = "operserv/spamfilter"; permission = "operserv/spamfilter"; }
** fantasy { name = "SPAMFILTER"; command = "operserv/spamfilter"; permission = "operserv/spamfilter"; prepend_channel = false; }
**
** Don't forget to add 'operserv/spamfilter' to your oper permissions
**/

class CommandOSSpamfilter : public Command
{
public:
	CommandOSSpamfilter(Module *creator) : Command(creator, "operserv/spamfilter", 1, 4)
	{
		this->SetDesc(_("Add spamfilter into IRCD config file"));
		this->SetSyntax("\037[Match Type]\037 \037[Target]\037 \037[Acttion]\037 \037[Match]\037 ");
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &matchtype = params[0];
		const Anope::string &target = params.size() > 1 ? params[1] : "";
		const Anope::string &action = params.size() > 2 ? params[2] : "";
		const Anope::string &match = params.size() > 3 ? params[3] : "";
        User *u = source.GetUser();

		if (!source.HasPriv("operserv/spamfilter"))
		{
			source.Reply(ACCESS_DENIED);
			return;
		}
		if (!matchtype.equals_ci("simple") || !matchtype.equals_ci("regex"))
		{
			source.Reply(_("Please choices \002simple\002 or \002regex\002"));
		    return;
		}
		if (!target.equals_ci("channel") || !target.equals_ci("private") || !target.equals_ci("private-notice") || !target.equals_ci("channel-notice")
		 || !target.equals_ci("part") || !target.equals_ci("quit") || !target.equals_ci("dcc") || !target.equals_ci("away")
		 || !target.equals_ci("topic") || !target.equals_ci("message-tag") || !target.equals_ci("user") || !target.equals_ci("default"))
		{
			source.Reply(_("\002Target\002 [ channel | private | private-notice | channel-notice | part | quit | dcc | away | topic | message-tag | user ]"));
			return;
		}
		if (!action.equals_ci("Tempshun") || !action.equals_ci("shun") || !action.equals_ci("kline") || !action.equals_ci("gline")
		 || !action.equals_ci("zline") || !action.equals_ci("gzline") || !action.equals_ci("kill") || !action.equals_ci("block")
		 || !action.equals_ci("dccblock") || !action.equals_ci("viruschan") || !action.equals_ci("warn"))
		{
			source.Reply(_("\002Action\002 [ Kill | Tempshun | shun | kline | gline | zline | gzline | block | dccblock | viruschan | warn ]"));
			return;
		}
		if (match.empty())
		{
			source.Reply(_("\002Match\002 specified string can be wildcard"));
		    return;
		}

	FILE *f = fopen(Config->GetBlock("spamfilter")->Get<const Anope::string>("config_path").c_str(), "a");
	if (f != NULL)
	{
		fprintf(f, "#Entry added by %s\n", u->nick.c_str());
		fprintf(f, "spamfilter {\n");
        fprintf(f, "	match-type %s;\n", matchtype.c_str());
        if (!target.equals_ci("default"))
		    { fprintf(f, "	target { %s; };\n", target.c_str()); }
        else
		    { fprintf(f, "	target { channel; private; private-notice; channel-notice; };\n"); }
        fprintf(f, "	action %s;\n", action.c_str());
		fprintf(f, "	match \"%s\";\n", match.c_str());
		Anope::string reason = Config->GetBlock("spamfilter")->Get<const Anope::string>("reason");
        fprintf(f, "	reason \"%s\";\n", reason.c_str());
		fprintf(f, "};\n");
		fclose(f);
		UplinkSocket::Message() << "REHASH -global";
		Log(LOG_ADMIN, source, this) << "to add [ " << match << " ] into spamfilter.conf ";
		u->SendMessage(Config->GetClient("OperServ"), "spamfilter adding the following URL: %s.", match.c_str());
	}
	else
		u->SendMessage(Config->GetClient("OperServ"), ":- Couldn't open spamfilter.conf! Aborting.!");
	}
	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{ 
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply("Spamfilter allows you to put spamfilter into Unrealircd spamfilter config block, "
					 "automatically rehash the server");
		source.Reply(" ");
		source.Reply("The command requires all 4 parameters.\n"
			     "\037Match-Type\037 - type of match string you're going to use Simple or Regex\n"
			     "\037Target\037 - the targets this spamfilter will look into like Private or Channel\n"
			     "\037Action\037 - specifies the action to be taken, such as kline.\n"
			     "\037Match\037 - the actual string that should be blocked or should perform  "
				 "the specified action on, this can be (wildcard) \n");
		return true;
	}
};

class OSSpamfilter : public Module
{
	CommandOSSpamfilter commandosspamfilter;
	
public:
	OSSpamfilter(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator),	
		commandosspamfilter(this)
	{
		if (!ModuleManager::FindModule("unreal") && !ModuleManager::FindModule("unreal4")) 
		{ 
			Log() << "ERROR: You are not running UnrealIRCd, this module only works on UnrealIRCd.";
			return;
		}
		this->SetAuthor("skutte");
		this->SetVersion(VER);
	}
};

MODULE_INIT(OSSpamfilter) 
