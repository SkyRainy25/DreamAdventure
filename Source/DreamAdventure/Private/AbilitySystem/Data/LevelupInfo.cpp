// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/LevelupInfo.h"

int32 ULevelupInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		if (LevelUpInformation.Num() - 1 <= Level)	return Level;
		
		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			Level++;
		}
		else
		{
			bSearching = false;
		}
	}

	return Level;
}
