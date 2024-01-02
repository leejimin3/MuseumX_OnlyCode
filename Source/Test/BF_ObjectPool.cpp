// Fill out your copyright notice in the Description page of Project Settings.


#include "BF_ObjectPool.h"

ABF_ObjectPool::ABF_ObjectPool()
{	
	BF = LoadClass<ABF_Object>(nullptr, TEXT("/Script/BF_Object"));
}

void ABF_ObjectPool::BeginPlay()
{
	Super::BeginPlay();
}

void ABF_ObjectPool::Init()
{
	for (int i = 0; i < 30; i++)
	{
		ABF_Object* BFObject = GetWorld()->SpawnActor<ABF_Object>(BF, FVector::ZeroVector, FRotator::ZeroRotator);

		BFObject->SetActorHiddenInGame(true);

		BFObject->SetActorEnableCollision(false);

		BFObject->SetActorTickEnabled(false);

		BF_Pool.Push(BFObject);
	}
}

ABF_Object* ABF_ObjectPool::SpawnObject()
{
	ABF_Object* BFObject = BF_Pool.Pop();

	BFObject->SetActorHiddenInGame(false);

	BFObject->SetActorEnableCollision(true);

	BFObject->SetActorTickEnabled(true);

	return BFObject;
}

void ABF_ObjectPool::ReturnObject(ABF_Object* BF_Object)
{
	BF_Object->SetActorHiddenInGame(true);

	BF_Object->SetActorEnableCollision(false);

	BF_Object->SetActorTickEnabled(false);

	BF_Pool.Push(BF_Object);
}

ABF_ObjectPool::~ABF_ObjectPool()
{

}
