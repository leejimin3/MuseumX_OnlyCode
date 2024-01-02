#include "BF_Object.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

ABF_Object::ABF_Object()
{
	PrimaryActorTick.bCanEverTick = true;

	RotationSpeed = 5.0f;
	MoveSpeed = 1.f;
	TargetScale = 170.0f;
	OutofScaleSpeed = 1.0f;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetGenerateOverlapEvents(true);

	Particle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Particle"));
	Particle->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Particle->SetVisibility(false);

	SoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SoundComponent"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/butterfly3/butterfly3__2_"));
	if (SkeletalMeshAsset.Succeeded()) { Mesh->SetSkeletalMesh(SkeletalMeshAsset.Object); }
}

void ABF_Object::Init()
{
	bMoving = false;

	Mesh->SetWorldScale3D(FVector(220.f, 220.0f, 220.0f));
	if (GetWorldTimerManager().IsTimerActive(ChangeScaleHandle)) GetWorldTimerManager().ClearTimer(ChangeScaleHandle);
	if (GetWorldTimerManager().IsTimerActive(CallFunctionHandle)) GetWorldTimerManager().ClearTimer(CallFunctionHandle);
	if (GetWorldTimerManager().IsTimerActive(OutitHandle)) GetWorldTimerManager().ClearTimer(OutitHandle);

	Dir = "RIGHT";
	MovementVector = GetActorLocation();

	FTimerHandle SpeedHandle;
	GetWorldTimerManager().SetTimer(SpeedHandle, [&]() { ChangeSpeed(); }, 5.0f, true);
	timerhandlelist.Add(SpeedHandle);

	USoundCue* SpawnSoundCue = LoadObject<USoundCue>(nullptr, TEXT("/Game/Sound/BtflyEntrance_Cue"));
	SoundComponent->SetSound(SpawnSoundCue);
	SoundComponent->Play();

	ChangeScale();
}

void ABF_Object::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bMoving) return;
	if (bisCalling) { BF_CallFunction(); return; }

	Move();
	ChangeDepth();
	Tracking(DeltaTime);
}

void ABF_Object::Move()
{
	if (Dir == "") return;

	if (Dir == "TOP")
	{
		MovementVector += FVector::UpVector * MoveSpeed;
	}
	else if (Dir == "BOTTOM")
	{
		MovementVector += FVector::DownVector * MoveSpeed;
	}
	else if (Dir == "LEFT")
	{
		MovementVector += FVector::BackwardVector * MoveSpeed;
	}
	else if (Dir == "RIGHT")
	{
		MovementVector += FVector::ForwardVector * MoveSpeed;
	}
}

void ABF_Object::SetDir(FString dir)
{
	if (bcantMove) return;
	Dir = dir;
}

void ABF_Object::Tracking(float DeltaTime)
{
	MovementVector.Y = FMath::Clamp(MovementVector.Y, CenterPos.Y - 500.f, CenterPos.Y + 0.f);

	FVector DirectionVector = (MovementVector - GetActorLocation()).GetSafeNormal();
	float newDirectionVectorPitch = FMath::RadiansToDegrees(FMath::Atan2(DirectionVector.X, DirectionVector.Z));
	FRotator LookAtDirection = DirectionVector.Rotation();

	if (FVector::Distance(GetActorLocation(), MovementVector) > 3.f)
	{
		SetActorLocation(FMath::Lerp(GetActorLocation(), MovementVector, DeltaTime));

		if (newDirectionVectorPitch > 0.f && newDirectionVectorPitch <= 90.f) newDirectionVectorPitch = FMath::Clamp(newDirectionVectorPitch, 3.f, 87.f);
		else if (newDirectionVectorPitch > 90.f && newDirectionVectorPitch <= 180.f) newDirectionVectorPitch = FMath::Clamp(newDirectionVectorPitch, 93.f, 177.f);
		else if (newDirectionVectorPitch <= 0.f && newDirectionVectorPitch > -90.f) newDirectionVectorPitch = FMath::Clamp(newDirectionVectorPitch, -87.f, -3.f);
		else if (newDirectionVectorPitch <= -90.f && newDirectionVectorPitch > -180.f) newDirectionVectorPitch = FMath::Clamp(newDirectionVectorPitch, -93.f, -176.f);

		if (newDirectionVectorPitch < 0.f)
		{
			if(LookAtDirection.Yaw > 0.f) LookAtDirection.Yaw = FMath::Clamp(LookAtDirection.Yaw, 120.f, 180.f);
			else if (LookAtDirection.Yaw < 0.f) LookAtDirection.Yaw = FMath::Clamp(LookAtDirection.Yaw, -180.f, -120.f);
			SetActorRotation(FMath::Lerp(GetActorRotation(), LookAtDirection + FRotator(0.f, 0.f, 270.f), DeltaTime * RotationSpeed));
		}
		else
		{
			LookAtDirection.Yaw = FMath::Clamp(LookAtDirection.Yaw, -60.f, 60.f);
			SetActorRotation(FMath::Lerp(GetActorRotation(), LookAtDirection + FRotator(0.f, 0.f, 90.f), DeltaTime * RotationSpeed));
		}

		Setlocation(GetActorLocation());
		Setrotation(GetActorRotation());
	}
	
}

void ABF_Object::ChangeSpeed()
{
	MoveSpeed = (float)FMath::RandRange(3, 5);
}

void ABF_Object::ChangeDepth()
{
	if (Dir == "" || bisMovingDepth) return;
	ChangeDepthTrigger();
	MovementVector.Y += DepthDir * MoveSpeed;
}

FBF_Struct ABF_Object::GetBF_Struct() { return bf_struct; }
void ABF_Object::SetBF_Struct(FBF_Struct _BF_Struct) { bf_struct = _BF_Struct; }

int ABF_Object::Getidx() { return bf_struct.Getidx(); }
void ABF_Object::Setidx(int32 _idx) { bf_struct.Setidx(_idx); }

int ABF_Object::Getformatidx() { return bf_struct.Getformatidx(); }
void ABF_Object::Setformatidx(int32 _format_idx) { bf_struct.Setformatidx(_format_idx); }

FString ABF_Object::Getbarcode() { return bf_struct.Getbarcode(); }
void ABF_Object::Setbarcode(FString _barcode) { bf_struct.Setbarcode(_barcode); }

FString ABF_Object::Getimg() { return bf_struct.Getimg(); }
void ABF_Object::Setimg(FString _img) { bf_struct.Setimg(_img); }

int ABF_Object::Getcolor() { return bf_struct.Getparticlecolor(); }
void ABF_Object::Setcolor(UNiagaraSystem* _particle, int _color)
{
	if (bf_struct.Getparticlecolor() == _color) { return; }

	Particle->SetAsset(_particle);
	Particle->Activate();
	bf_struct.Setparticlecolor(_color);
}

FVector ABF_Object::Getlocation() { return bf_struct.Getlocation(); }
void ABF_Object::Setlocation(FVector _location)
{
	SetActorLocation(_location);
	bf_struct.Setlocation(_location);
}

FRotator ABF_Object::Getrotation() { return bf_struct.Getrotation(); }
void ABF_Object::Setrotation(FRotator _rotation)
{
	SetActorRotation(_rotation);
	bf_struct.Setrotation(_rotation);
}

void ABF_Object::BF_Call()
{
	if(bisCalling || bcantMove) return;
	bisCalling = true;
}



void ABF_Object::ChangeDepthTrigger()
{
	bisMovingDepth = true;
	DepthDir = FMath::RandRange(-100, 100);
	FTimerHandle timerhandle;
	GetWorldTimerManager().SetTimer(timerhandle, [&]() { bisMovingDepth = false; }, 0.016f, false, 1.f);
}

void ABF_Object::ChangeScale()
{
	GetWorldTimerManager().SetTimer(ChangeScaleHandle, [&]() {

		FVector TargetVector = FVector(TargetScale, TargetScale, TargetScale);
		SetActorScale3D(FMath::Lerp(GetActorScale3D(), TargetVector, 3.0f * GetWorld()->GetDeltaSeconds()));

		if (FMath::IsNearlyEqual(GetActorScale3D().X, TargetScale, 3.0f))
		{
			SetActorScale3D(FVector(TargetScale, TargetScale, TargetScale));

			Mesh->SetGenerateOverlapEvents(true);
			bMoving = true;
			Mesh->SetCollisionProfileName("OverlapAllDynamic");
			Particle->SetVisibility(true);

			USoundCue* SpawnSoundCue = LoadObject<USoundCue>(nullptr, TEXT("/Game/Sound/BtflyFlying_Cue"));
			SoundComponent->SetSound(SpawnSoundCue);
			SoundComponent->Play();

			GetWorldTimerManager().ClearTimer(ChangeScaleHandle);
		}

		}, 0.016f, true, 2.0f);
}

void ABF_Object::ChangeDir(FString dir)
{
	bcantMove = true;
	Mesh->SetGenerateOverlapEvents(false);

	MovementVector = GetActorLocation() + MovementVector.Normalize();
	MoveSpeed = 1.5f;
	RotationSpeed /= 5.f;

	Dir = dir;

	FTimerHandle timerhandle;
	GetWorldTimerManager().SetTimer(timerhandle, [&]() { bcantMove = false; RotationSpeed *= 5.f; Mesh->SetGenerateOverlapEvents(true); }, 0.016f, false, 2.0f);
}

void ABF_Object::SetMaterial(UTexture2D* wingtexture, UTexture2D* masktexture)
{
	UMaterialInterface* MaterialTemplate = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/M_DissolveStart"));
	MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(MaterialTemplate, nullptr);
	MaterialInstanceDynamic->SetTextureParameterValue(FName("B1"), wingtexture);
	MaterialInstanceDynamic->SetTextureParameterValue(FName("Opacity"), masktexture);

	if (MaterialInstanceDynamic != NULL && MaterialInstanceDynamic != nullptr) Mesh->SetMaterial(1, MaterialInstanceDynamic);
}

void ABF_Object::BF_CallFunction()
{
	if(bisroutine) return;
	if (CallNum == 0)
	{
		bisroutine = true;
		Mesh->SetGenerateOverlapEvents(false);
		CurrentPos = GetActorLocation();

		FVector aroundPoint = GetActorLocation();
		aroundPoint.X = (CenterPos.X + aroundPoint.X) / 2.f;
		aroundPoint.Y = CenterPos.Y + 330.f;
		aroundPoint.Z = (CenterPos.Z + aroundPoint.Z) / 2.f;

		FVector center = (CurrentPos + aroundPoint) / 2.f;
		MoveToPoint(center);
	}
	else if (CallNum == 1)
	{
		bisroutine = true;
		FVector aroundPoint = CurrentPos;

		FVector center = (GetActorLocation() + aroundPoint) / 2.f;
		FTimerHandle timerhandle;
		GetWorldTimerManager().SetTimer(timerhandle, [&, center]() { MoveToPoint(center); }, 0.016f, false, 3.f );
	}
	else if (CallNum == 2)
	{
		CallNum = 0;
		Mesh->SetGenerateOverlapEvents(true);
		bisCalling = false;
	}
}

void ABF_Object::MoveToPoint(FVector PointLocation)
{
	dimensions = GetActorLocation() - PointLocation;
	AxisVector = CheckSpinRotation(PointLocation);
	Multiplier = 150.f;
	CallNum == 0 ? AngleMax = 180.f : AngleMax = -180.f;
	AngleAxis = 0.f;
	CenterPoint = PointLocation;
	
	GetWorldTimerManager().SetTimer(CallFunctionHandle, [&]() {
		FVector NewLocation = CenterPoint;
		
		if (CallNum == 0)
		{
			AngleAxis += GetWorld()->GetDeltaSeconds() * Multiplier;
			if (AngleAxis >= AngleMax)
			{
				CompleteMoveToPoint();
				return;
			}
		}

		else
		{
			AngleAxis -= GetWorld()->GetDeltaSeconds() * Multiplier;
			if (AngleAxis <= AngleMax)
			{
				CompleteMoveToPoint();
				return;
			}
		}

		FVector RotateValue = dimensions.RotateAngleAxis(AngleAxis, AxisVector);

		NewLocation.X += RotateValue.X; 
		NewLocation.Y += RotateValue.Y;	
		NewLocation.Z += RotateValue.Z;

		FRotator NewRotation = (NewLocation - GetActorLocation()).Rotation();
		
		SetActorLocation(NewLocation);
		SetActorRotation(NewRotation);

		Setlocation(NewLocation);
		Setrotation(NewRotation);

	}, 0.016f, true);
}

void ABF_Object::CompleteMoveToPoint()
{
	SetActorRotation(FRotator(90.f, -90.f, 0.f));
	CallNum++;
	bisroutine = false;
	Particle->SetVisibility(!(Particle->GetVisibleFlag()));
	GetWorldTimerManager().ClearTimer(CallFunctionHandle);
	return;
}

FVector ABF_Object::CheckSpinRotation(FVector PointLocation)
{
	FVector SpinRotation = FVector(0.f, 0.f, 0.f);

	SpinRotation.Z = PointLocation.X < CenterPos.X ? 1.f : -1.f;
	SpinRotation.X = PointLocation.Z < CenterPos.Z ? -0.5f : 0.5f;

	return SpinRotation;
}

void ABF_Object::Outit()
{
	for (FTimerHandle T : timerhandlelist)
	{
		GetWorldTimerManager().ClearTimer(T);
	}

	if (GetWorldTimerManager().IsTimerActive(ChangeScaleHandle)) GetWorldTimerManager().ClearTimer(ChangeScaleHandle);
	if (GetWorldTimerManager().IsTimerActive(CallFunctionHandle)) GetWorldTimerManager().ClearTimer(CallFunctionHandle);

	OutofScale = TargetScale;
	GetWorldTimerManager().SetTimer(OutitHandle, [&]() {
		Mesh->SetWorldScale3D(FVector(OutofScale, OutofScale, OutofScale));
		OutofScale -= OutofScaleSpeed;
		if (OutofScale < 3.0f)
		{
			Mesh->SetWorldScale3D(FVector(0.f, 0.f, 0.f));
			GetWorldTimerManager().ClearTimer(OutitHandle);

			FTimerHandle timerhandle;
			GetWorldTimerManager().SetTimer(timerhandle, [&]() {

				//Mesh->SetWorldScale3D(FVector(TargetScale, TargetScale, TargetScale));
				//Dir = "";
				Particle->SetVisibility(false);

				}, 0.016f, false, 3.0f);
		}

		}, 0.016f, true);
}

//void ABF_Object::Outit()
//{
//	for (FTimerHandle T : timerhandlelist)
//	{
//		GetWorldTimerManager().ClearTimer(T);
//	}
//
//	FTimerHandle timerhandle;
//	DynamicHandle = timerhandle;
//	OutofScale = TargetScale;
//	GetWorldTimerManager().SetTimer(DynamicHandle, [&]() {
//		Mesh->SetWorldScale3D(FVector(OutofScale, OutofScale, OutofScale));
//		OutofScale -= OutofScaleSpeed;
//		if (OutofScale < 1.0f)
//		{
//			Mesh->SetWorldScale3D(FVector(0.f, 0.f, 0.f));
//
//			FTimerHandle timerhandle;
//			GetWorldTimerManager().SetTimer(DynamicHandle, [&](){  
//			
//			OutofScale = 100.f;
//			Mesh->SetWorldScale3D(FVector(OutofScale, OutofScale, OutofScale)); 
//			Dir = "";
//			Particle->SetVisibility(false);
//			
//			}, 0.016f, false, 3.0f);
//		}
//
//	}, 0.016f, true);
//}