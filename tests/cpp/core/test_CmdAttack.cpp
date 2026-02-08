#include "Property.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "GameTypes.h"

#include <gtest/gtest.h>

// Expose private members of CmdAttack for testing purposes only
#define private public
#include "commands/CmdAttack.h"
#undef private

#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompArmor.h"
#include "components/CompAttack.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompVision.h"

using game::ArmorClass;

namespace core
{

class CmdAttackTest : public ::testing::Test, public core::PropertyInitializer
{
  protected:
    void SetUp() override
    {
        // Register a fresh StateManager for each test
        auto sm = std::make_shared<StateManager>();
        ServiceRegistry::getInstance().registerService(sm);
        m_stateMan = sm;

        // Create an entity and attach required components (UnitComponentRefs expects these)
        m_entity = m_stateMan->createEntity();
        m_stateMan->addComponent<CompAction>(m_entity, CompAction());
        m_stateMan->addComponent<CompAnimation>(m_entity, CompAnimation());
        m_stateMan->addComponent<CompEntityInfo>(m_entity, CompEntityInfo(0));
        m_stateMan->addComponent<CompPlayer>(m_entity, CompPlayer());
        m_stateMan->addComponent<CompTransform>(m_entity, CompTransform());
        m_stateMan->addComponent<CompUnit>(m_entity, CompUnit());
        m_stateMan->addComponent<CompVision>(m_entity, CompVision());

        // Add CompAttack (will be configured by each test individually)
        m_stateMan->addComponent<CompAttack>(m_entity, CompAttack());

        // Create CmdAttack instance and initialize so m_components points to entity components
        m_cmd = std::make_unique<CmdAttack>();
        m_cmd->setEntityID(m_entity);
        m_cmd->init();
    }

    void TearDown() override
    {
        // Clear state manager registry contents to avoid cross-test interference
        m_stateMan->clearAll();
    }

    // Helpers to configure attack/armor properties
    void setAttackPerClass(const std::vector<int>& attacks)
    {
        auto& compAttack = m_stateMan->getComponent<CompAttack>(m_entity);
        PropertyInitializer::set<std::vector<int>>(compAttack.attackPerClass, attacks);
    }

    void setAttackMultiplierPerClass(const std::vector<float>& mults)
    {
        auto& compAttack = m_stateMan->getComponent<CompAttack>(m_entity);
        PropertyInitializer::set<std::vector<float>>(compAttack.attackMultiplierPerClass, mults);
    }

    static void setArmorPerClass(CompArmor& armor, const std::vector<int>& values)
    {
        PropertyInitializer::set<std::vector<int>>(armor.armorPerClass, values);
    }

    static void setDamageResistance(CompArmor& armor, float rr)
    {
        PropertyInitializer::set<float>(armor.damageResistance, rr);
    }

    Ref<StateManager> m_stateMan;
    uint32_t m_entity = entt::null;
    std::unique_ptr<CmdAttack> m_cmd;
};

// 1. attacker's class has matching armor class in target with value greater than zero
TEST_F(CmdAttackTest, MatchingArmorClass_ReducesDamage)
{
    // Attack vector: only MELEE (index 1) has attack 10
    setAttackPerClass(std::vector<int>{0, 10, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    setArmorPerClass(target, std::vector<int>{0, 5, 0}); // MELEE armor = 5
    setDamageResistance(target, 0.0f);

    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 5.0f); // 10 - 5 = 5
}

// 2. attacker's class has not matching armor class in target -> baseArmor applied (simulate high
// base armor)
TEST_F(CmdAttackTest, NoMatchingClass_UsesBaseArmor_ResultsInMinimumDamage)
{
    setAttackPerClass(std::vector<int>{10, 0, 0}); // INFANTRY (index 0) attack
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    // Simulate 'baseArmor' effect by giving a very large armor for class 0
    setArmorPerClass(target, std::vector<int>{1000, 0, 0});
    setDamageResistance(target, 0.0f);

    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 1.0f); // totalDamage would be 0 -> min final damage 1
}

// 3. matching armor class with value zero -> attack is applied fully
TEST_F(CmdAttackTest, MatchingArmorZero_AttackApplied)
{
    setAttackPerClass(std::vector<int>{0, 10, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    setArmorPerClass(target, std::vector<int>{0, 0, 0}); // MELEE armor = 0
    setDamageResistance(target, 0.0f);

    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 10.0f);
}

// 4. attacker has attack multiplier
TEST_F(CmdAttackTest, AttackMultiplier_AppliesToDamage)
{
    setAttackPerClass(std::vector<int>{0, 10, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.5f, 1.0f}); // MELEE multiplier 1.5

    CompArmor target;
    setArmorPerClass(target, std::vector<int>{0, 5, 0}); // MELEE armor 5
    setDamageResistance(target, 0.0f);

    // multipliedAttack = 10 * 1.5 = 15 -> 15 - 5 = 10
    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 10.0f);
}

// 5. target has damage resistance
TEST_F(CmdAttackTest, DamageResistance_ReducesFinalDamage)
{
    setAttackPerClass(std::vector<int>{0, 10, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    setArmorPerClass(target, std::vector<int>{0, 0, 0});
    setDamageResistance(target, 0.2f); // 20% damage resistance

    // totalDamage = 10 -> final = max(1, 10 * (1-0.2)) = 8
    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 8.0f);
}

// 6. target armor has negative armor hence attack is increased
TEST_F(CmdAttackTest, NegativeArmor_IncreasesDamage)
{
    setAttackPerClass(std::vector<int>{0, 10, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    setArmorPerClass(target, std::vector<int>{0, -2, 0}); // negative MELEE armor
    setDamageResistance(target, 0.0f);

    // damage = 10 - (-2) = 12
    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 12.0f);
}

// 7. ensure damage is not negative even per class (per-class floored to 0)
TEST_F(CmdAttackTest, PerClassDamageFlooredToZero_FinalDamageMin1)
{
    setAttackPerClass(std::vector<int>{0, 3, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    setArmorPerClass(target, std::vector<int>{0, 5, 0}); // MELEE armor > attack
    setDamageResistance(target, 0.0f);

    // per-class damage = max(0, 3 - 5) = 0 -> total 0 -> final min 1
    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 1.0f);
}

// 8. target has armor but attacker doesn't have attack for that class (attacker's zero attack
// should not subtract)
TEST_F(CmdAttackTest, TargetHasArmor_AttackerNoAttackForThatClass_Ignored)
{
    // Attacker only has MELEE attack
    setAttackPerClass(std::vector<int>{0, 10, 0});
    setAttackMultiplierPerClass(std::vector<float>{1.0f, 1.0f, 1.0f});

    CompArmor target;
    // INFANTRY and PIERCE have armor but attacker has 0 attack for these classes
    setArmorPerClass(target, std::vector<int>{50, 5, 20});
    setDamageResistance(target, 0.0f);

    // Only MELEE contributes: 10 - 5 = 5
    float dmg = m_cmd->getDamage(target);
    EXPECT_FLOAT_EQ(dmg, 5.0f);
}
} // namespace core