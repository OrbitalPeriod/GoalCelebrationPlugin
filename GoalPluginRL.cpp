#include "pch.h"
#include "GoalPluginRL.h"
#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h> 

/*
bakkesmod goal celebration plugin

requires SSH library
via SSH, enter command to trigger bot on external server
*/
BAKKESMOD_PLUGIN(GoalPluginRL, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

using namespace std;

void GoalPluginRL::onLoad()
{
	_globalCvarManager = cvarManager;
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&GoalPluginRL::onGameEnd, this));
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&GoalPluginRL::GameStart, this));
	cvarManager->log("Plugin loaded!");


	enabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("GoalPluginRL_ENABLED", "0", "Enable the GoalPluginRL plugin", true, true, 0, true, 1).bindTo(enabled);
}

void GoalPluginRL::onUnload()
{
	cvarManager->log("Plugin unloaded!");
}

void GoalPluginRL::GameStart() {
	if (!gameWrapper->IsInOnlineGame()) return;

	cvarManager->log("Online game started");

	CarWrapper me = gameWrapper->GetLocalCar();
	if (me.IsNull()) return;

	PriWrapper mePRI = me.GetPRI();
	if (mePRI.IsNull()) return;

	TeamInfoWrapper myTeam = mePRI.GetTeam();
	if (myTeam.IsNull()) return;

	// Get TeamNum
	myTeamNum = myTeam.GetTeamNum();

	// Set Game Started
	isGameStarted = true;
	isGameEnded = false;
}

void GoalPluginRL::onGameEnd() {

	ServerWrapper server = gameWrapper->GetOnlineGame();
	TeamWrapper winningTeam = server.GetGameWinner();
	if (winningTeam.IsNull()) return;
	if (!gameWrapper->IsInOnlineGame() && myTeamNum != -1) return;

	cvarManager->log("Online game ended");

	int teamnum = winningTeam.GetTeamNum();
	isGameEnded = true;
	if (myTeamNum == winningTeam.GetTeamNum()) {
		LogEv("Game won");
		onWin();
	}
	else
	{
		LogEv("Game loss");
	}
	myTeamNum = -1;

}

void GoalPluginRL::onWin() {
	ssh_session my_ssh_session;
	int rc;
	int port;
	int verbosity = SSH_LOG_PROTOCOL;
	char* password;
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL) {
		cvarManager->log("SSH connection not established");
		return;
	}
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "<IP>");
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "<UserName>");
	ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	rc = ssh_connect(my_ssh_session);

	if (rc != SSH_OK) {
		cvarManager->log("SSH connection not established");
		return;
	}
	password = "<Password>";
	rc = ssh_userauth_password(my_ssh_session, NULL, password);
	if (rc != SSH_AUTH_SUCCESS) {
		cvarManager->log("SSH password incorrect");
		return;
	}

	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);
}

void GoalPluginRL::LogEv(std::string inp) {
	cvarManager->log(inp);
}