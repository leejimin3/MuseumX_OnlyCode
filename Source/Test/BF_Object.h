#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BF_Struct.h"
#include "NiagaraComponent.h"
#include "BF_Object.generated.h"

UCLASS()
class TEST_API ABF_Object : public AActor
{
	GENERATED_BODY()

public:
	ABF_Object();

protected:

private:
	void ChangeScale();

	UPROPERTY(EditAnywhere, Category = "Mesh")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Particle")
		UNiagaraComponent* Particle;

	UPROPERTY(EditAnywhere, Category = "Sound")
		UAudioComponent* SoundComponent;

	UPROPERTY(EditAnywhere, Category = "collision")
		class USphereComponent* collision;

	UMaterialInstanceDynamic* MaterialInstanceDynamic;

	UPROPERTY(EditAnywhere, Category = State)
		float RotationSpeed;

	UPROPERTY(EditAnywhere, Category = State)
		float MoveSpeed;

	UPROPERTY(EditAnywhere, Category = State)
		float TargetScale;

	UPROPERTY(EditAnywhere, Category = State)
		float OutofScaleSpeed;

	FTimerHandle ChangeScaleHandle;
	FTimerHandle CallFunctionHandle;
	FTimerHandle OutitHandle;

	float TargetTime;
	float CurrentTime;

	FRotator TargetRot;

	FString Dir;
	bool bcantMove;

	void Move();

	void ChangeDepth(); void ChangeDepthTrigger();

	void ChangeSpeed();

	void Tracking(float DeltaTIme);

	void MoveToPoint(FVector PointLocation);

	void BF_CallFunction();

	void CompleteMoveToPoint();

	FVector MovementVector;

	bool bisMovingDepth; int DepthDir;

	bool bisCalling; bool bisroutine; bool bisSpining;
	int CallNum;

	FVector CurrentPos;
	FVector dimensions;
	FVector AxisVector;
	FVector CenterPoint;

	float Multiplier;
	float AngleMax;
	float AngleAxis;

	float OutofScale;

	FVector CheckSpinRotation(FVector PointLocation);

	TArray<FTimerHandle> timerhandlelist;

	FBF_Struct bf_struct;

public:



	void Init();

	void Outit();

	virtual void Tick(float DeltaTime) override;

	FVector CenterPos;

	UFUNCTION(BlueprintCallable)
		void ChangeDir(FString dir);

	UPROPERTY(BlueprintReadWrite, Category = "Accel")
		bool bMoving;

	UPROPERTY(BlueprintReadWrite, Category = "Call")
		bool bCalling;

	int32 Getidx(); void Setidx(int32 _idx);
	int32 Getformatidx(); void Setformatidx(int32 _format_idx);
	FString Getbarcode(); void Setbarcode(FString _barcode);
	FString Getimg(); void Setimg(FString _img);
	int Getcolor(); void Setcolor(UNiagaraSystem* _particle, int _color);
	FVector Getlocation(); void Setlocation(FVector _location);
	FRotator Getrotation(); void Setrotation(FRotator _rotation);
	FBF_Struct GetBF_Struct(); void SetBF_Struct(FBF_Struct _BF_Struct);

	void SetMaterial(UTexture2D* wingtexture, UTexture2D* masktexture);

	void SetDir(FString dir);

	void BF_Call();
};