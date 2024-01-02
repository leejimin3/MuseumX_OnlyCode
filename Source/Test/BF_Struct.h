#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "NiagaraSystem.h"
//#include "Particles/ParticleSystem.h"

struct FBF_Struct
{
private:
	int32 idx;	// ������ȣ
	int32 format_idx; // ����ũ �̹���
	FString barcode; // ���ڵ�
	FString img; // ���� �̹���

	int particlecolor; // ��ƼŬ ��

	FVector location; // ���� ��ġ��
	FRotator rotation; // ���� ȸ����

public:
	int32 Getidx() const {return idx;} 
	void Setidx(int32 _idx) {idx = _idx;}

	int32 Getformatidx() const { return format_idx; }
	void Setformatidx(int32 _format_idx) { format_idx = _format_idx; }

	FString Getbarcode() const {return barcode;}
	void Setbarcode(FString _barcode) {barcode = _barcode ;}

	FString Getimg() const {return img;} 
	void Setimg(FString _img) {img = _img;}

	int Getparticlecolor() const { return particlecolor;}
	void Setparticlecolor(int _particlecolor) { particlecolor = _particlecolor;}

	FVector Getlocation() const { return location; }
	void Setlocation(FVector _location) { location = _location; }

	FRotator Getrotation() const { return rotation; }
	void Setrotation(FRotator _rotation) { rotation = _rotation; }
};
