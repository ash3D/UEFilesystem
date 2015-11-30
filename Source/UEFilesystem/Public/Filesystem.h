#pragma once

#include "GameFramework/Actor.h"
#include "Filesystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(Filesystem, Log, All);

UCLASS()
class AFilesystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFilesystem();

public:
	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	void CreateDirectory(const FString &path) const;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	void Remove(const FString &path, bool force = true) const;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	void Rename(const FString &oldPath, const FString &newPath) const;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	FString CurrentPath() const;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	FString GamePath() const;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	FString GameDir(bool forceAbsolute) const;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	void Print(const FString &path) const;
};
