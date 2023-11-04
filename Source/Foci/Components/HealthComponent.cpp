#include "HealthComponent.h"

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UHealthComponent::AddHealth(float Health)
{
	if (Health < 0.0f && CurrentHealth > 0.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("HealthComponent taken damage."));
		OnTakeDamage.Broadcast();
	}
	if (Health + CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	else if (Health + CurrentHealth <= 0.0f)
	{
		if (CurrentHealth > 0.0f)
		{
			UE_LOG(LogTemp, Display, TEXT("Enemy died."));
			OnDeath.Broadcast();
		}
		CurrentHealth = 0.0f;
	}
	else
	{
		CurrentHealth += Health;
	}
	const float HealthPercentage = MaxHealth == 0.0f ? 0.0f : CurrentHealth / MaxHealth;
	// UE_LOG(LogTemp, Display, TEXT("%s's health changed, broadcasting delegate from Health Component"), *GetOwner()->GetFName().ToString())
	OnHealthChanged.Broadcast(CurrentHealth, HealthPercentage);
	return CurrentHealth;
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	if (CurrentHealth > NewMaxHealth)
	{
		CurrentHealth = NewMaxHealth;
	}
	MaxHealth = NewMaxHealth;
	OnMaxHealthChanged.Broadcast(NewMaxHealth);
}

bool UHealthComponent::IsAlive() const
{
	return CurrentHealth > 0.0f;
}
