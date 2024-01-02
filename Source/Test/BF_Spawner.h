#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BF_Object.h"
#include "BF_ObjectPool.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "IWebSocket.h"
#include "NiagaraSystem.h"
#include "BF_Spawner.generated.h"

UCLASS()
class TEST_API ABF_Spawner : public AActor
{
	GENERATED_BODY()

public:
	ABF_Spawner();

private:

	FHttpModule* Http;
	TSharedPtr<IWebSocket> WebSocket;
	
	ABF_ObjectPool* ObjectPool;
	TArray<class ABF_Object*> BF_List;
	ABF_Object* SelectedBF;

	UPROPERTY(EditAnywhere, Category = "State")
		float Range;
	UPROPERTY(EditAnywhere, Category = "State")
		FRotator SpawnRotation;

	UPROPERTY(EditAnywhere, Category = "ObjectPool")
		TSubclassOf<class ABF_ObjectPool> BF_ObjectPool;

	UPROPERTY(EditAnywhere, Category = "Particle")
		TArray<UNiagaraSystem*> ParticleList;

	UPROPERTY(EditAnywhere, Category = "Particle")
		TArray<UTexture2D*> MaskList;

	UPROPERTY(EditAnywhere, Category = "Particle")
		TArray<UNiagaraSystem*> SpawnParticleList;

	void MakeSocket();

	void ReStartSocket();

	void CheckProtocol(const FString& MessageString);

	void BUTTERFLY_ADD(TSharedPtr<FJsonObject> JsonObject, bool bisloadBF);

	void BUTTERFLY_MOVE(TSharedPtr<FJsonObject> JsonObject);

	void BUTTERFLY_REMOVE(TSharedPtr<FJsonObject> JsonObject);

	void BUTTERFLY_CALL(TSharedPtr<FJsonObject> JsonObject);

	void WEB_ACTIVE_BUTTERFLY_LIST(TSharedPtr<FJsonObject> JsonObject);

	void SetMaterial(const FString& InURL, FBF_Struct FStruct, FVector SpawnLocation);

	void GetImage(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FBF_Struct FStruct, FVector SpawnLocation);

	void MakeBF(UTexture2D* AITexture, FBF_Struct FStruct, FVector SpawnLocation);

	UTexture2D* LoadTextureFromdata(const TArray<uint8>& Data);


	FString SerializedJson(TSharedPtr<FJsonObject> JsonObject);

	FString MakeErrorJson(FString message);

	FString MakeProtocolJson(FString message);

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
