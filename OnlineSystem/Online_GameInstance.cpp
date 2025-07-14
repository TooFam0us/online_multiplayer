// Fill out your copyright notice in the Description page of Project Settings.


#include "Online_GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include <Online/OnlineSessionNames.h>
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

UOnline_GameInstance::UOnline_GameInstance()
{
	MySessionName = FName("My Session");
}

void UOnline_GameInstance::Init()
{
	Super::Init();
	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get())
	{
		SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Binding Delegates
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UOnline_GameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UOnline_GameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UOnline_GameInstance::OnJoinSessionComplete);
			SessionInterface->OnUpdateSessionCompleteDelegates.AddUObject(this, &UOnline_GameInstance::OnUpdateSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UOnline_GameInstance::OnSessionUserInviteAccepted);
			SessionInterface->OnSessionInviteReceivedDelegates.AddUObject(this, &UOnline_GameInstance::OnSessionInviteReceived);
			SessionInterface->OnSessionParticipantLeftDelegates.AddUObject(this, &UOnline_GameInstance::OnSessionParticipantLeft);
		}
	}
}

void UOnline_GameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete, success: %d"), bWasSuccessful);
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(SelectedMapURL + "?listen");
	}
}

void UOnline_GameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	SearchingForServer.Broadcast(false);

	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete, success: %d"), bWasSuccessful);
	if (bWasSuccessful)
	{
		int32 ArrayIndex = -1;
		for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
		{
			++ArrayIndex;
			if (!Result.IsValid())
				continue;


			FServerInfo Info;

			FString ServerName = "Empty Server Name";

			Result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName);

			Info.ServerName = ServerName;
			Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;
			Info.ServerArrayIndex = ArrayIndex;
			Info.Ping = Result.PingInMs;
			Info.SetPlayerCount();
			ServerListDel.Broadcast(Info);
		}

	}
}

void UOnline_GameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete"));
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			FString JoinAddress = "";
			SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
			if (JoinAddress != "")
			{
				PC->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);

			}
		}
	}
	else 
	{
		EOnJoinSessionResult res;
		switch (Result)
		{
			case EOnJoinSessionCompleteResult::AlreadyInSession:
				res = EOnJoinSessionResult::AlreadyInSession;
				ServerJoinedResult.Broadcast(res);
				break;
			case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
				res = EOnJoinSessionResult::CouldNotRetrieveAddress;
				ServerJoinedResult.Broadcast(res);
				break;
			case EOnJoinSessionCompleteResult::SessionDoesNotExist :
				res = EOnJoinSessionResult::SessionDoesNotExist;
				ServerJoinedResult.Broadcast(res);
				break;
			case EOnJoinSessionCompleteResult::SessionIsFull:
				res = EOnJoinSessionResult::SessionIsFull;
				ServerJoinedResult.Broadcast(res);
				break;
			case EOnJoinSessionCompleteResult::UnknownError:
			default:
				res = EOnJoinSessionResult::UnknownError;
				ServerJoinedResult.Broadcast(res);
				break;
		}
	}

}

void UOnline_GameInstance::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnUpdateSessionComplete"));
}

void UOnline_GameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	SessionInterface->JoinSession(0, MySessionName, InviteResult);
}

void UOnline_GameInstance::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	SessionInterface->JoinSession(0, MySessionName, InviteResult);
}
void UOnline_GameInstance::OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& PlayerId, EOnSessionParticipantLeftReason Reason)
{
	EOnSessionLeftReason res = EOnSessionLeftReason::Closed;
	res = Reason;
	ServerLeftReason.Broadcast(res);
}

void UOnline_GameInstance::CreateServer(FCreateServerInfo ServerInfo, bool bPrivateSession)
{
	UE_LOG(LogTemp, Warning, TEXT("Creating server"));

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;

	// SET TO USE SERVER INFO FOR LAN IN FUTURE IF NEED BE
	UE_LOG(LogTemp, Warning, TEXT("Subsystem in use: %s"), *IOnlineSubsystem::Get()->GetSubsystemName().ToString());

	SessionSettings.bIsLANMatch = false;

	SessionSettings.bShouldAdvertise = bPrivateSession==false;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = ServerInfo.MaxPlayers;

	SessionSettings.BuildUniqueId = 1;
	SessionSettings.bUseLobbiesIfAvailable = true;

	SessionSettings.Set(FName("SERVER_NAME_KEY"), ServerInfo.ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


	SessionInterface->CreateSession(0, MySessionName, SessionSettings);
}

void UOnline_GameInstance::UpdateServer(FCreateServerInfo ServerInfo, bool bAdvertiseServer)
{
	FOnlineSessionSettings SessionSettings;

	SessionSettings.bShouldAdvertise = bAdvertiseServer;

	SessionSettings.NumPublicConnections = ServerInfo.MaxPlayers;
	SessionInterface->UpdateSession(MySessionName, SessionSettings);
}

void UOnline_GameInstance::FindServers()
{
	SearchingForServer.Broadcast(true);

	UE_LOG(LogTemp, Warning, TEXT("Finding servers"));

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 1000000;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UOnline_GameInstance::JoinServer(int32 ArrayIndex)
{
	FOnlineSessionSearchResult Result = SessionSearch->SearchResults[ArrayIndex];
	if (Result.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("JOINING SERVER AT INDEX: %d"), ArrayIndex);
		SessionInterface->JoinSession(0, MySessionName, Result);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FAILED TO JOIN SERVER AT INDEX: %d"), ArrayIndex);
	}
}

void UOnline_GameInstance::LeaveServer()
{
	SessionInterface->DestroySession(MySessionName);
}


void UOnline_GameInstance::SetSelectedMap(FString MapName)
{
	SelectedMapURL = MapName;
}
