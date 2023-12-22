// Fill out your copyright notice in the Description page of Project Settings.


#include "TestWidget.h"
#include "STestWidget.h"

TSharedRef<SWidget> UTestWidget::RebuildWidget()
{
	return SNew(STestWidget);
}
