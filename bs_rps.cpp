/*
************************************************************************************************
** Module	: bs_rps.cpp
** Version	: 1.0.1
** Author	: skutte (SkyChatz1234@gmail.com)
** Release	: 11th July, 2025
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
** /msg BotServ RPS #channel START <rounds>
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
#include <map>

struct RPSGameState {
    int rounds;
    int played;
    int user_wins;
    int bot_wins;
};

class CommandBSRps : public Command
{
    std::map<std::string, RPSGameState> games; 

 public:
    CommandBSRps(Module* creator, const Anope::string& sname = "gameserv/rps") : Command(creator, sname, 1, 3)
    {
        this->SetDesc(_("Perform a Rock paper scissors Games"));
        this->SetSyntax("\037#channel\037 \037Action\037 [rounds]");
        this->AllowUnregistered(true);
    }

    void Execute(CommandSource& source, const std::vector<Anope::string>& params) anope_override
{
    std::vector<Anope::string> responses;
    responses.push_back("Rock");
    responses.push_back("Paper");
    responses.push_back("Scissors");

    if (params.size() < 2)
    {
        source.Reply("Syntax: RPS #channel START <rounds> or RPS #channel <ROCK|PAPER|SCISSORS>");
        return;
    }

    const Anope::string &chan = params[0];
    const Anope::string &action = params[1];
    int total_rounds = 1;
    if (params.size() > 2)
        total_rounds = atoi(params[2].c_str());
    if (total_rounds < 1) total_rounds = 1;

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

    std::string key = chan.c_str();
    key += ":";
    key += source.GetNick().c_str();

    // Handle game start
    if (action.equals_ci("START"))
    {
        if (games.find(key) != games.end())
        {
            source.Reply("You already have a game in progress in this channel!");
            return;
        }
        RPSGameState state;
        state.rounds = total_rounds;
        state.played = 0;
        state.user_wins = 0;
        state.bot_wins = 0;
        games[key] = state;
        IRCD->SendPrivmsg(*ci->bi, ci->name, "%s: The Rock-Paper-Scissors game will begin now! Type ROCK, PAPER, or SCISSORS to play.", source.GetNick().c_str());
        return;
    }

    // Only allow moves if a game is started
    if (games.find(key) == games.end())
    {
        source.Reply("You must start a game first: !RPS START <rounds>");
        return;
    }

    // Validate move
    Anope::string user_move = action;
    if (!user_move.equals_ci("ROCK") && !user_move.equals_ci("PAPER") && !user_move.equals_ci("SCISSORS"))
    {
        source.Reply("Please choose ROCK, PAPER, or SCISSORS.");
        return;
    }

    RPSGameState &state = games[key];

    int pick = rand() % responses.size();
    Anope::string text = responses[pick];

    if ((user_move.equals_ci("ROCK") && text.equals_ci("SCISSORS")) ||
        (user_move.equals_ci("PAPER") && text.equals_ci("ROCK")) ||
        (user_move.equals_ci("SCISSORS") && text.equals_ci("PAPER")))
    {
        state.user_wins++;
        IRCD->SendPrivmsg(*ci->bi, ci->name, "You Win this round! %s vs %s.", user_move.c_str(), text.c_str());
    }
    else if ((user_move.equals_ci("ROCK") && text.equals_ci("PAPER")) ||
             (user_move.equals_ci("PAPER") && text.equals_ci("SCISSORS")) ||
             (user_move.equals_ci("SCISSORS") && text.equals_ci("ROCK")))
    {
        state.bot_wins++;
        IRCD->SendPrivmsg(*ci->bi, ci->name, "Bot Wins this round! %s vs %s.", text.c_str(), user_move.c_str());
    }
    else
    {
        IRCD->SendPrivmsg(*ci->bi, ci->name, "Tie this round! Both chose %s.", user_move.c_str());
    }

    state.played++;

    if (state.played >= state.rounds)
    {
        // Game over, announce winner and kick if user lost
        if (state.user_wins > state.bot_wins)
        {
            IRCD->SendPrivmsg(*ci->bi, ci->name, "%s wins the game! (%d:%d)", source.GetNick().c_str(), state.user_wins, state.bot_wins);
        }
        else if (state.user_wins < state.bot_wins)
        {
            IRCD->SendPrivmsg(*ci->bi, ci->name, "Bot wins the game! (%d:%d) %s will be kicked.", state.bot_wins, state.user_wins, source.GetNick().c_str());
            User *u = User::Find(source.GetNick().c_str());
            if (u)
                ci->c->Kick(ci->bi, u, "You lost the RPS game!");
        }
        else
        {
            IRCD->SendPrivmsg(*ci->bi, ci->name, "The game is a tie! (%d:%d)", state.user_wins, state.bot_wins);
        }
        games.erase(key);
    }
    else
    {
        IRCD->SendPrivmsg(*ci->bi, ci->name, "Round %d/%d complete. Score: You %d - Bot %d", state.played, state.rounds, state.user_wins, state.bot_wins);
    }
}

    bool OnHelp(CommandSource& source, const Anope::string& subcommand) anope_override
    {
        this->SendSyntax(source);
        source.Reply(" ");
        source.Reply(_("To play Rock-Paper-Scissors with GameServ:"));
        source.Reply(_("1. Start a game: RPS #channel START <rounds>"));
        source.Reply(_("   Example: RPS #channel START 3"));
        source.Reply(_("2. After the bot announces the game has started, play your move:"));
        source.Reply(_("   RPS #channel ROCK or RPS #channel PAPER or RPS #channel SCISSORS"));
        source.Reply(_("The game will keep score for the specified number of rounds."));
        source.Reply(_("If you lose, you will be kicked from the channel. If you win or tie, you stay!"));
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
	}
};

MODULE_INIT(BSRps)
