#include "module.h"
#define VER "1.0.2"

/**
************************************************************************************************
** Module : os_spamfilter.cpp
** Version : 1.0.2
** Author : skutte (SkyChatz1234@gmail.com)
** Release : 5th May, 2023
** Update : 17th July, 2025
** Github : https://github.com/SkyChatz/anope-modules
************************************************************************************************
** <Description>
**
** This module is based on spamfilter.c by JeffK https://modules.anope.org/index.php?page=view&id=262
** Rewritten for Anope V2 compatibility.
**
** Recommended: Use latest Anope and UnrealIRCd versions.
**
************************************************************************************************
** Tested: Anope 2.0.12, UnrealIRCd 6+
**
** Commands:
** /msg OperServ HELP SPAMFILTER
** /msg OperServ SPAMFILTER [Match Type] [Target] [Action] [Match]
** Fantasy: !spamfilter simple channel kill *badword*
**
** Configuration:
** module { name = "os_spamfilter"; }
** spamfilter {
**     config_path = "/home/ircd/spamfilter.conf";
**     reason = "No Spam/Advertising On Our Network";
**     input_conversion = "strip-control-codes";
**     ban_time = "1d";
**     show_message = "channel-only";
**     except_mask = "*!*@example.com";
** }
** command { service = "OperServ"; name = "SPAMFILTER"; command = "operserv/spamfilter"; permission = "operserv/spamfilter"; }
** fantasy { name = "SPAMFILTER"; command = "operserv/spamfilter"; permission = "operserv/spamfilter"; prepend_channel = false; }
**/

class CommandOSSpamfilter : public Command
{
public:
    CommandOSSpamfilter(Module *creator) : Command(creator, "operserv/spamfilter", 1, 4)
    {
        this->SetDesc(_("Add spamfilter into IRCD config file"));
        this->SetSyntax("\037[Match Type]\037 \037[Target]\037 \037[Action]\037 \037[Match]\037");
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

        if (!matchtype.equals_ci("simple") && !matchtype.equals_ci("regex"))
        {
            source.Reply(_("Match-Type must be \002simple\002 or \002regex\002"));
            return;
        }

        if (!target.equals_ci("channel") && !target.equals_ci("private") && !target.equals_ci("private-notice") &&
            !target.equals_ci("channel-notice") && !target.equals_ci("part") && !target.equals_ci("quit") &&
            !target.equals_ci("dcc") && !target.equals_ci("away") && !target.equals_ci("topic") &&
            !target.equals_ci("message-tag") && !target.equals_ci("user") && !target.equals_ci("default"))
        {
            source.Reply(_("\002Target\002 must be one of: channel | private | private-notice | channel-notice | part | quit | dcc | away | topic | message-tag | user | default"));
            return;
        }

        if (!action.equals_ci("Tempshun") && !action.equals_ci("shun") && !action.equals_ci("kline") &&
            !action.equals_ci("gline") && !action.equals_ci("zline") && !action.equals_ci("gzline") &&
            !action.equals_ci("kill") && !action.equals_ci("block") && !action.equals_ci("dccblock") &&
            !action.equals_ci("viruschan") && !action.equals_ci("warn"))
        {
            source.Reply(_("\002Action\002 must be one of: Kill | Tempshun | shun | kline | gline | zline | gzline | block | dccblock | viruschan | warn"));
            return;
        }

        if (match.empty())
        {
            source.Reply(_("\002Match\002 string cannot be empty"));
            return;
        }

        // Load optional config values
        ConfigReader reader;
        Anope::string config_path = reader.ReadValue("spamfilter", "config_path", 0);
        Anope::string reason = reader.ReadValue("spamfilter", "reason", 0, false, "Spam is not allowed");
        Anope::string input_conversion = reader.ReadValue("spamfilter", "input_conversion", 0, false, "strip-control-codes");
        Anope::string ban_time = reader.ReadValue("spamfilter", "ban_time", 0, false, "1d");
        Anope::string show_message = reader.ReadValue("spamfilter", "show_message", 0, false, "channel-only");
        Anope::string except_mask = reader.ReadValue("spamfilter", "except_mask", 0);

        FILE *f = fopen(config_path.c_str(), "a");
        if (f != NULL)
        {
            fprintf(f, "# Entry added by %s\n", u->nick.c_str());
            fprintf(f, "spamfilter {\n");
            fprintf(f, "    match-type %s;\n", matchtype.c_str());
            fprintf(f, "    match \"%s\";\n", match.c_str());
            fprintf(f, "    input-conversion %s;\n", input_conversion.c_str());
            fprintf(f, "    target { %s; };\n", target.equals_ci("default") ? "channel; private; private-notice; channel-notice" : target.c_str());
            fprintf(f, "    action %s;\n", action.c_str());
            fprintf(f, "    reason \"%s\";\n", reason.c_str());
            fprintf(f, "    ban-time %s;\n", ban_time.c_str());
            fprintf(f, "    show-message-content-on-hit %s;\n", show_message.c_str());
            if (!except_mask.empty())
                fprintf(f, "    except { %s; };\n", except_mask.c_str());
            fprintf(f, "};\n\n");
            fclose(f);

            UplinkSocket::Message() << "REHASH -global";
            Log(LOG_ADMIN, source, this) << "Added spamfilter for match: " << match;
            u->SendMessage(Config->GetClient("OperServ"), "Spamfilter added for match: %s", match.c_str());
        }
        else
        {
            u->SendMessage(Config->GetClient("OperServ"), ":- Couldn't open spamfilter.conf! Aborting.");
            Log(LOG_ERROR) << "Failed to open spamfilter.conf at path: " << config_path;
        }
    }

    bool OnHelp(CommandSource &source, const Anope::string &subcommand)
    {
        this->SendSyntax(source);
        source.Reply(" ");
        source.Reply("Adds a spamfilter entry to UnrealIRCd's config and rehashes the server.");
        source.Reply(" ");
        source.Reply("Required:");
        source.Reply("  \037Match-Type\037 - simple or regex");
        source.Reply("  \037Target\037 - channel, private, etc.");
        source.Reply("  \037Action\037 - kill, kline, warn, etc.");
        source.Reply("  \037Match\037 - string to match (wildcards allowed)");
        source.Reply(" ");
        source.Reply("Optional values are read from the config block 'spamfilter'.");
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
            Log() << "ERROR: UnrealIRCd not detected. This module only works with UnrealIRCd.";
            return;
        }
        this->SetAuthor("skutte");
        this->SetVersion(VER);
    }
};

MODULE_INIT(OSSpamfilter)
