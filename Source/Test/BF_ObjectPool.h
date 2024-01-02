// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BF_Object.h"
#include "BF_ObjectPool.generated.h"

UCLASS()
class TEST_API ABF_ObjectPool : public AActor
{
	GENERATED_BODY()

public:

	void Init();

	virtual void BeginPlay() override;

	ABF_ObjectPool();

	UFUNCTION()
	ABF_Object* SpawnObject();

	UFUNCTION()
	void ReturnObject(ABF_Object* BF_Object);



private:
	TArray<ABF_Object*> BF_Pool;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABF_Object> BF;

	~ABF_ObjectPool();
};
