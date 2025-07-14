# Steam OnlineSubsystem Integration

A minimal Unreal Engine C++ `UGameInstance` implementation that leverages the OnlineSubsystem Steam to create, find, join, and leave multiplayer sessions.  
Includes Blueprint‐assignable delegates for UI wiring and a reference `AMultiplayerCharacter` showing basic replication patterns.

---

## Features

- Create or update a Steam session with a custom name, max players, visibility  
- Discover active sessions and stream results to Blueprint  
- Join or leave sessions and handle travel  
- React to Steam invites and participant leaves  
- Reference pawn demonstrating RPCs and property replication  

---

## Requirements

- Unreal Engine 4.26 or higher (UE5 compatible)  
- Enabled plugins:  
  - OnlineSubsystem  
  - OnlineSubsystemUtils  
  - OnlineSubsystemSteam  
- Valid `SteamAppId.txt` placed in your `Binaries/Win64/`  
- Steam SDK configured per official docs  

---

## Setup

1. **Engine config**  
   In `Config/DefaultEngine.ini`:
   ```ini
   [OnlineSubsystem]
   DefaultPlatformService=Steam

   [OnlineSubsystemSteam]
   SteamDevAppId=480
   bEnabled=true
   ```
2. **Steam AppID** Place your `SteamAppId.txt` (containing your AppID) into:
  ```YourProject/Binaries/Win64/```

3. **Module dependencies** In your `YourModule.Build.cs`:
    ```
    PublicDependencyModuleNames.AddRange(new string[]{
    "Core", "Engine",
    "OnlineSubsystem",
    "OnlineSubsystemUtils",
    "OnlineSubsystemSteam"
    });
    ```
    
  4. **Game Instance** In Project Settings → Maps & Modes, set Game Instance Class to `Online_GameInstance`.


--- 

## Usage

### C++ Example

```
// Grab your custom GameInstance
UOnline_GameInstance* GI = Cast<UOnline_GameInstance>(GetGameInstance());

// 1. Create or update a session
FCreateServerInfo Info;
Info.ServerName    = TEXT("My Steam Lobby");
Info.MaxPlayers    = 8;
Info.bVisibleLobby = true;
GI->CreateServer(Info, /*bPrivate=*/ false);

// 2. Search for sessions
GI->FindServers();

// 3. Join a selected session (by array index)
GI->JoinServer(SelectedIndex);

// 4. Leave the active session
GI->LeaveServer();
```

---

Blueprint Flow

  1. Get Online_GameInstance reference.

  2. Call nodes:

        - CreateServer

        - UpdateServer

        - FindServers

        - JoinServer

        - LeaveServer

        - SetSelectedMap

  3. Bind to delegates:

        - ServerListDel (streams back FServerInfo)

        - SearchingForServer (bool for spinner)

        - ServerJoinedResult (enum success/errors)

        - ServerLeftReason (enum leave/disconnect/kick/close)

  ---

## API Reference

| Function                                 | Delegate                   | Description                                      |
|------------------------------------------|----------------------------|--------------------------------------------------|
| `CreateServer(FCreateServerInfo, bool)`  |                            | Create a new Steam session                       |
| `UpdateServer(FCreateServerInfo, bool)`  |                            | Update max players & lobby visibility            |
| `FindServers()`                          | `SearchingForServer`       | Begin async session search                       |
| `JoinServer(int32 ArrayIndex)`           | `ServerJoinedResult`       | Join the indexed session                         |
| `LeaveServer()`                          | `ServerLeftReason`         | Destroy or leave the current session             |
| `SetSelectedMap(FString MapName)`        |                            | Choose map travel URL before session start       |

---

## MultiplayerCharacter Reference

This actor is not a full demo—just a template for networked pawns:

  - Enable replication: `bReplicates = true;`

  - Implement `GetLifetimeReplicatedProps()` with `DOREPLIFETIME_WITH_PARAMS_FAST`

  - Use Server RPCs (`Server_OnFire`) for client→server calls

  - Use NetMulticast RPCs (`Multi_OnFire`) to broadcast to all clients

  - RepNotify (`OnRep_TakeDamage`, `OnRep_SpendStamina`) for property updates

---

## Contributing

Feel free to file issues or submit pull requests. Suggestions for extended session settings, UI bindings or Steam features are welcome!
