/*
************************************************************************************************
** Module	: bs_rps.cpp
** Version	: 1.0.0
** Author	: skutte (SkyChatz1234@gmail.com)
** Release	: 20th September, 2022
** Update	: -
************************************************************************************************
** <Description>
**
**		This module is designed for ppl who wanted to play rps games
**
**		It is recommended to use latest Anope version 
**
** </Description>
**
************************************************************************************************
** Tested: 2.0.11, Unreal6+
**
** Providing Command:
**
** /msg BotServ HELP RPS
** /msg BotServ RPS #channel rock | paper | scissors 
** or
** fantasy
** !rps rock | paper | scissors  
**
** Contact me 
** Email : SkyChatz1234@hotmail.com
** IRC : /s irc.skychatz.org:7000 #modules
**
** You may use this module under the terms of docs/LICENSE, in the Anope source directory
**
** Configuration:
** module { name = "bs_rps"; }
** command { name = "RPS"; service = "BotServ"; command = "botserv/rps"; }
** fantasy { name = "RPS"; command = "botserv/rps"; prepend_channel = true; }
 */

#include "module.h"

class CommandBSRps : public Command
{
 public:
	CommandBSRps(Module* creator, const Anope::string& sname = "botserv/rps") : Command(creator, sname, 1, 2)
	{
		this->SetDesc(_("Perform a Rock paper scissors Games"));
		this->SetSyntax("\037#channel\037 \037Action\037");
		this->AllowUnregistered(true);
	}

	void Execute(CommandSource& source, const std::vector<Anope::string>& params) anope_override
	{
		std::vector<Anope::string> responses;
		responses.reserve(25);
		responses.push_back("Paper");
		responses.push_back("Rock");
		responses.push_back("Scissors");

		const Anope::string &chan = params[0];
		const Anope::string &action = params.size() > 1 ? params[1] : "";
		ChannelInfo* ci = ChannelInfo::Find(chan);

		if (!ci)
		{
			source.Reply(CHAN_X_NOT_REGISTERED, chan.c_str());
			return;
		}

		if (!ci->bi)
		{
			source.Reply(BOT_NOT_ASSIGNED);
			return;
		}

		if (!ci->c || !ci->c->FindUser(ci->bi))
		{
			source.Reply(BOT_NOT_ON_CHANNEL, ci->name.c_str());
			return;
		}

		int pick = rand() % responses.size();
		Anope::string text = responses[pick];
		if(action.empty())
		{
			source.Reply("Please Choise ROCK | PAPER | SCISSORS");
			return;
		}

		if ((action.equals_ci("ROCK") && text.equals_ci("PAPER")))
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "%s Wins! Paper wraps Rock.", ci->bi->nick.c_str());
		}

		else if ((action.equals_ci("PAPER") && text.equals_ci("SCISSORS")))
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "%s Wins! Scissors cut Paper.",ci->bi->nick.c_str());
		}
		
		else if ((action.equals_ci("SCISSORS") && text.equals_ci("ROCK")))
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "%s Wins! Rock smashes Scissors.",ci->bi->nick.c_str());
		}

		else if ((action.equals_ci("ROCK") && text.equals_ci("SCISSORS")))
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "You Win! Rock smashes Scissors.");
		}

		else if ((action.equals_ci("ROCK") && text.equals_ci("PAPER")))
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "You Win! Paper wraps Rock.");
		}

		else if ((action.equals_ci("SCISSORS") && text.equals_ci("PAPER")))
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "You Win! Scissors cut Paper.");
		}

		else 
		{
			IRCD->SendPrivmsg(*ci->bi, ci->name, "Tie. Play again win the Game.");
		}
	}

	bool OnHelp(CommandSource& source, const Anope::string& subcommand) anope_override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Rock-Paper-Scissors is a game played to settle disputes between two people. \n"
		 			   "Thought to be a game of chance that depends on random luck similar to \n"
					   "flipping coins or drawing straws \n "
		 			   ""));
		return true;
	}
};

class BSRps : public Module
{
	CommandBSRps commandbsrps;

 public:
	BSRps(const Anope::string& modname, const Anope::string& creator) : Module(modname, creator, THIRD),
		commandbsrps(this)
	{
		if(Anope::VersionMajor() < 2)
		{
			throw ModuleException("Requires version 2.x.x of Anope.");
		}
		this->SetAuthor("skutte");
		this->SetVersion("1.0.0");
	}
};

MODULE_INIT(BSRps)
