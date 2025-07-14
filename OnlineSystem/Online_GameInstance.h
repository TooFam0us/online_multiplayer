// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online_GameInstance.generated.h"


USTRUCT(BlueprintType)
struct FCreateServerInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString ServerName;
	UPROPERTY(BlueprintReadWrite)
	int32 MaxPlayers;
	UPROPERTY(BlueprintReadWrite)
	bool bVisibleLobby;
};

USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FString ServerName;
	UPROPERTY(BlueprintReadOnly)
	FString PlayerCountStr;
	UPROPERTY(BlueprintReadOnly)
	int32 Ping;
	UPROPERTY(BlueprintReadOnly)
	int32 ServerArrayIndex;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly)
	bool bIsVisibleLobby;

	void SetPlayerCount()
	{
		PlayerCountStr = FString(FString::FromInt(CurrentPlayers) + "/" + FString::FromInt(MaxPlayers));
	}
};

UENUM(BlueprintType)
enum EOnJoinSessionResult 
{
	Success,
	SessionIsFull,
	SessionDoesNotExist,
	CouldNotRetrieveAddress,
	AlreadyInSession,
	UnknownError
};

UENUM(BlueprintType)
enum EOnSessionLeftReason
{
	/** The participant left the session of their own accord */
	Left,
	/** The participant got disconnected from a session that is still active */
	Disconnected,
	/** The participant was forcefully removed from the session */
	Kicked,
	/** The session ended and the participant got removed from it */
	Closed
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerDel, FServerInfo, ServerListDel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerSearchingDel, bool, bSearchingForServer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerJoinedResultDel, EOnJoinSessionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerLeftReasonDel, EOnSessionLeftReason, Reason);
/**
 *
 */
UCLASS()
class SPLATSPLAT_API UOnline_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UOnline_GameInstance();

protected:

	FName MySessionName;

	FString SelectedMapURL;

	UPROPERTY(BlueprintAssignable)
	FServerDel ServerListDel;

	UPROPERTY(BlueprintAssignable)
	FServerSearchingDel SearchingForServer;

	UPROPERTY(BlueprintAssignable)
	FServerJoinedResultDel ServerJoinedResult;

	UPROPERTY(BlueprintAssignable)
	FServerLeftReasonDel ServerLeftReason;


	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionComplete(bool bWasSuccessful);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	virtual void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	virtual void OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& PlayerId, EOnSessionParticipantLeftReason Reason);

	UFUNCTION(BlueprintCallable)
	void CreateServer(FCreateServerInfo ServerInfo, bool bPrivateSession);

	UFUNCTION(BlueprintCallable)
	void UpdateServer(FCreateServerInfo ServerInfo, bool bAdvertiseServer);

	UFUNCTION(BlueprintCallable)
	void FindServers();

	UFUNCTION(BlueprintCallable)
	void JoinServer(int32 ArrayIndex);

	UFUNCTION(BlueprintCallable)
	void LeaveServer();

	UFUNCTION(BlueprintCallable)
	void SetSelectedMap(FString MapName);

};
