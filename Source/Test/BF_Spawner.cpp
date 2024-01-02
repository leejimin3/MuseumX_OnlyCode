#include "BF_Spawner.h"
#include "Kismet/KismetMathLibrary.h"
#include "ImageWrapper/Public/IImageWrapper.h"
#include "ImageWrapper/Public/IImageWrapperModule.h"
#include "WebSocketsModule.h"
#include "NiagaraFunctionLibrary.h"

ABF_Spawner::ABF_Spawner()
{
	SpawnRotation = FRotator(90.f, -90.f, 0.f);
	Range = 300.f;
}

void ABF_Spawner::BeginPlay()
{
	Super::BeginPlay();

	Http = &FHttpModule::Get();
	MakeSocket();
	FTimerHandle timerhandle;
	GetWorldTimerManager().SetTimer(timerhandle, [&]() { if(!(WebSocket->IsConnected())) WebSocket->Connect(); }, 0.5f, true, 1.f);

	if (!ObjectPool)
	{
		ObjectPool = GetWorld()->SpawnActor<ABF_ObjectPool>(BF_ObjectPool, FVector::ZeroVector, FRotator::ZeroRotator);
		ObjectPool->Init();
	}
}

void ABF_Spawner::MakeSocket()
{
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets")) { FModuleManager::Get().LoadModule("WebSockets"); }

	WebSocket = FWebSocketsModule::Get().CreateWebSocket("xxxxxx://xxxxxx.xxxxxx.xx:xxxxx/", "xxxxxx"); //url 보호를 위해 x로 표시

	if (WebSocket != NULL)
	{
		WebSocket->OnConnected().AddLambda([]()
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Successfully connected!");
			});

		WebSocket->OnConnectionError().AddLambda([this](const FString& Error)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Error);
			});

		WebSocket->OnClosed().AddLambda([&](int32 StatueCode, const FString& Reason, bool bWasClean)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, bWasClean ? FColor::Green : FColor::Red, "Connention closed " + Reason);
			});

		WebSocket->OnMessage().AddLambda([this](const FString& MessageString)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, "Received message: " + MessageString);
				CheckProtocol(MessageString);
			});

		WebSocket->OnMessageSent().AddLambda([](const FString& MessageString)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Sent message: " + MessageString);
			});

		WebSocket->Connect();
	}
}

void ABF_Spawner::ReStartSocket()
{
	FTimerHandle timerhandle;
	GetWorldTimerManager().SetTimer(timerhandle, [&]() { if (WebSocket == NULL) MakeSocket(); }, 0.016f, false, 3.f);
}

void ABF_Spawner::CheckProtocol(const FString& MessageString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(MessageString);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (JsonObject->HasField("protocol"))
		{
			FString Protocol = JsonObject->GetStringField("protocol");

			if (Protocol == "BUTTERFLY_ADD")
			{
				BUTTERFLY_ADD(JsonObject, false);
			}
			else if (Protocol == "BUTTERFLY_MOVE")
			{
				BUTTERFLY_MOVE(JsonObject);
			}
			else if (Protocol == "BUTTERFLY_REMOVE")
			{
				BUTTERFLY_REMOVE(JsonObject);
			}
			else if (Protocol == "BUTTERFLY_CALL")
			{
				BUTTERFLY_CALL(JsonObject);
			}
			else if (Protocol == "WEB_ACTIVE_BUTTERFLY_LIST")
			{
				WEB_ACTIVE_BUTTERFLY_LIST(JsonObject);
			}
			else
			{
				WebSocket->Send(MakeErrorJson("Not Collect Protocol !"));
			}
		}
	}
}

void ABF_Spawner::MakeBF(UTexture2D* AITexture, FBF_Struct FStruct, FVector SpawnLocation)
{
	ABF_Object* BFobject = ObjectPool->SpawnObject();

	BFobject->Setidx(FStruct.Getidx());
	BFobject->Setformatidx(FStruct.Getformatidx());
	BFobject->Setbarcode(FStruct.Getbarcode());
	BFobject->Setimg(FStruct.Getimg());
	BFobject->Setcolor(ParticleList[FStruct.Getparticlecolor() - 1], FStruct.Getparticlecolor());
	BFobject->SetMaterial(AITexture, MaskList[FStruct.Getformatidx() - 1]);
	BFobject->CenterPos = GetActorLocation();

	BFobject->SetActorLocation(SpawnLocation);
	BFobject->SetActorRotation(SpawnRotation);

	BFobject->Setlocation(SpawnLocation);
	BFobject->Setrotation(SpawnRotation);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpawnParticleList[FStruct.Getformatidx() - 1], BFobject->GetActorLocation() + FVector(0.f, 20.f, 0.f), SpawnRotation, FVector(1.1f, 1.1f, 1.1f), true, true);

	BF_List.Add(BFobject);
	BFobject->Init();

	WebSocket->Send(MakeErrorJson("BUTTERFLY_ADD : Success"));

}

void ABF_Spawner::BUTTERFLY_ADD(TSharedPtr<FJsonObject> JsonObject, bool bisloadBF)
{
	TSharedPtr<FJsonObject> DataObject;
	if (!bisloadBF)
	{
		if (!JsonObject->HasField("data")) { WebSocket->Send(MakeErrorJson("Invalid Data Field!"));  return; }
		DataObject = JsonObject->GetObjectField("data");
	}
	else
	{
		DataObject = JsonObject;
	}

	if (!DataObject->HasField("but_idx")) { WebSocket->Send(MakeErrorJson("Invalid but_idx Field!"));  return; }
	if (!DataObject->HasField("but_format")) { WebSocket->Send(MakeErrorJson("Invalid but_format Field!"));  return; }
	if (!DataObject->HasField("barcode")) { WebSocket->Send(MakeErrorJson("Invalid barcode Field!"));  return; }
	if (!DataObject->HasField("but_img")) { WebSocket->Send(MakeErrorJson("Invalid but_img Field!"));  return; }

	FBF_Struct FStruct;
	FStruct.Setidx(StaticCast<int32>(DataObject->GetIntegerField("but_idx")));
	FStruct.Setformatidx(StaticCast<int32>(DataObject->GetIntegerField("but_format")));
	FStruct.Setbarcode((DataObject->GetStringField("barcode")));
	FStruct.Setimg(DataObject->GetStringField("but_img"));
	FStruct.Setparticlecolor(StaticCast<int32>(FMath::RandRange(1, 3)));

	FString but_img = FStruct.Getimg();
	FVector SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), FVector(Range));

	SetMaterial(but_img, FStruct, SpawnLocation);
}

void ABF_Spawner::BUTTERFLY_MOVE(TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject->HasField("data")) { WebSocket->Send(MakeErrorJson("Invalid Data Field!"));  return; }

	TSharedPtr<FJsonObject> DataObject = JsonObject->GetObjectField("data");

	if (!DataObject->HasField("but_idx")) { WebSocket->Send(MakeErrorJson("Invalid but_idx Field!"));  return; }
	if (!DataObject->HasField("move")) { WebSocket->Send(MakeErrorJson("Invalid move Field!"));  return; }

	if (DataObject.IsValid())
	{
		int32 idx = StaticCast<int32>(DataObject->GetIntegerField("but_idx"));

		for (ABF_Object* BF_idx : BF_List)
		{
			if (BF_idx->Getidx() == idx)
			{
				FString move = DataObject->GetStringField("move");

				BF_idx->SetDir(move);
				return;
			}
		}
	}
}

void ABF_Spawner::BUTTERFLY_REMOVE(TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject->HasField("data")) { WebSocket->Send(MakeErrorJson("Invalid Data Field!"));  return; }

	TSharedPtr<FJsonObject> DataObject = JsonObject->GetObjectField("data");

	if (DataObject.IsValid())
	{
		if (!DataObject->HasField("but_idx")) { WebSocket->Send(MakeErrorJson("Invalid but_idx Field!"));  return; }

		int32 idx = StaticCast<int32>(DataObject->GetIntegerField("but_idx"));

		for (ABF_Object* BF_idx : BF_List)
		{
			if (BF_idx->Getidx() == idx)
			{
				BF_List.Remove(BF_idx);
				BF_idx->Outit();

				SelectedBF = BF_idx;
				FTimerHandle timerhandle;
				GetWorldTimerManager().SetTimer(timerhandle, [&]() { ObjectPool->ReturnObject(SelectedBF); }, 0.016f, false, 3.0f);

				WebSocket->Send(MakeErrorJson("BUTTERFLY_REMOVE : Success"));
				return;
			}
		}
		WebSocket->Send(MakeErrorJson("BUTTERFLY_REMOVE : Can't find but_idx butterfly!"));
	}
}

void ABF_Spawner::BUTTERFLY_CALL(TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject->HasField("data")) { WebSocket->Send(MakeErrorJson("Have not Data Field!"));  return; }

	TSharedPtr<FJsonObject> DataObject = JsonObject->GetObjectField("data");

	if (DataObject.IsValid())
	{
		if (!DataObject->HasField("but_idx")) { WebSocket->Send(MakeErrorJson("Invalid but_idx Field!"));  return; }

		int32 idx = StaticCast<int32>(DataObject->GetIntegerField("but_idx"));

		for (ABF_Object* BF_idx : BF_List)
		{
			if (BF_idx->Getidx() == idx)
			{
				BF_idx->BF_Call();
				WebSocket->Send(MakeErrorJson("BUTTERFLY_CALL : Success"));
				return;
			}
		}
		WebSocket->Send(MakeErrorJson("BUTTERFLY_CALL : Can't find but_idx butterfly!"));
	}
}

void ABF_Spawner::WEB_ACTIVE_BUTTERFLY_LIST(TSharedPtr<FJsonObject> JsonObject)
{
	if (JsonObject->HasField("data"))
	{
		const TArray<TSharedPtr<FJsonValue>>& DataArray = JsonObject->GetArrayField("data");
		for (const TSharedPtr<FJsonValue>& DataValue : DataArray)
		{
			TSharedPtr<FJsonObject> DataObject = DataValue->AsObject();

			FTimerHandle T;
			float RandomValue = FMath::FRandRange(0.0f, 5.0f);
			GetWorldTimerManager().SetTimer(T, [=]() { BUTTERFLY_ADD(DataObject, true); }, RandomValue, false);
		}
	}
}

void ABF_Spawner::SetMaterial(const FString& InURL, FBF_Struct FStruct, FVector SpawnLocation)
{
	TSharedPtr<IHttpRequest> Request = Http->CreateRequest();

	Request->SetURL(InURL);
	Request->SetVerb("Get");
	Request->OnProcessRequestComplete().BindUObject(this, &ABF_Spawner::GetImage, FStruct, SpawnLocation);
	Request->ProcessRequest();
}

void ABF_Spawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
}






FString ABF_Spawner::MakeErrorJson(FString message)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField("message", message);

	return SerializedJson(JsonObject);
}

FString ABF_Spawner::MakeProtocolJson(FString message)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField("protocol", message);

	return SerializedJson(JsonObject);
}

FString ABF_Spawner::SerializedJson(TSharedPtr<FJsonObject> JsonObject)
{
	FString SerializedJson;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&SerializedJson);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	JsonWriter->Close();
	return SerializedJson;
}



void ABF_Spawner::GetImage(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FBF_Struct FStruct, FVector SpawnLocation)
{
	TArray<uint8> ResponseData = Response->GetContent();
	FString ResponseStr = FString(UTF8_TO_TCHAR(ResponseData.GetData()));
	UE_LOG(LogTemp, Log, TEXT("Response received: %s"), *ResponseStr);

	TArray<uint8> TextureData;
	FString ContentType = Response->GetContentType();

	if (ContentType == "image/png") { TextureData = ResponseData; UE_LOG(LogTemp, Warning, TEXT("Success!! Content type: %s"), *ContentType); }

	else { UE_LOG(LogTemp, Warning, TEXT("Unexpected content type: %s"), *ContentType); return; }

	UTexture2D* Texture = LoadTextureFromdata(TextureData);
	if (Texture)
	{
		Texture->UpdateResource();
		MakeBF(Texture, FStruct, SpawnLocation);
		UE_LOG(LogTemp, Warning, TEXT("Texture Success!"));

	}
	else { UE_LOG(LogTemp, Warning, TEXT("Failed to load texture from data!")); }
}

UTexture2D* ABF_Spawner::LoadTextureFromdata(const TArray<uint8>& Data)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(Data.GetData(), Data.Num());

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("Failed to create image wrapper!")); return nullptr; }

	ImageWrapper->SetCompressed(Data.GetData(), Data.Num());

	TArray<uint8> UncompressedData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get uncompressed data from image wrapper!")); return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
	if (!Texture) { UE_LOG(LogTemp, Warning, TEXT("Failed to create texture!")); return nullptr; }

	Texture->UpdateResource();

	void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, UncompressedData.GetData(), UncompressedData.Num());
	Texture->PlatformData->Mips[0].BulkData.Unlock();

	Texture->CompressionSettings = TC_Default;
	Texture->Filter = TextureFilter::TF_Bilinear;

	return Texture;
}