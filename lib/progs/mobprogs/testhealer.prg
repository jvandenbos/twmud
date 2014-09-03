#include ../include/string

>greet_prog(100, ch=$n, me=$t) {
  %obj = 3635
  %chName = GetName(ch)

  // Check to see if char is an Ice Haven Guard
  if(!HasObj(ch, obj)) {
    [
     'I see that you are not an Ice Haven Guard.
     'If you wish for my help, visit the Guard's Office.
     'You can find the office just East of my shop.
    ]
  }
}

>speech_prog(list, ch=$n, me=$t, sp=$s)
{
     %obj = 3635
     %chName = GetName(ch)
     %sp = Trim(sp) // get rid of any white space around the text
     
     //make sure it is the word we are searching for, and no other
     if((sp != "list") && (HasObj(ch, obj)))
     {
      DoWith(me)
      {
           [
             'Devotion to The Holy Mother takes great consintration.
             'Please don't waste my time asking things that I can not do.
           ]
      }
     }

   if((sp == "list") && (HasObj(ch, obj)))
   {
      MakeString("st")
     {
         [
           -=Services of the Holy Mother=-
             refresh, mana
             cure light, cure serious, cure critic, heal
             cure blind, remove poison, remove curse
             armor, sanctuary, shield, stone skin
             strength, bless, regen, invis, fly, waterbreath
             create light, sense aura, sense life, true sight, infra, detect invis
             energy ward, electric ward, acid ward, fire ward, cold ward
             stone fist, dispel magic
             succor, knowledge
         ]
      }
      SendToChar(st, ch)
    }
}


>command_prog(buy, ch=$n, me=%t, arg=%s)
{
   //Command prog to buy spells from the healer

   //variables for price...so if you want to update the price, you don't
   //have to scroll through the entire thing

   %prRefresh = 1000
   %prMana = 5000
   %prCureLight = 2016
   %prCureSerious = 2967
   %prCureCritic = 4145
   %prHeal = 5000
   %prCureBlind = 1000
   %prRemovePoison = 1000
   %prRemoveCurse = 1000
   %prArmor = 16500
   %prSanctuary = 16500
   %prShield = 16500
   %prStoneSkin = 16500
   %prStrength = 500
   %prBless = 20000
   %prRegen = 8000
   %prInvis = 10000
   %prFly = 16500
   %prCreateLight = 500
   %prSenseAura = 16500
   %prSenseLife = 16500
   %prTrueSight = 16500
   %prInfra = 16500
   %prDetectInvis = 16500
   %prEnergyWard = 16500
   %prElectricWard = 16500
   %prAcidWard = 16500
   %prFireWard = 16500
   %prColdWard = 16500
   %prStoneFist = 50000
   %prDispel = 1000
   %prSuccor = 3679
   %prKnowledge = 3679

   %arg = ToUpper(Trim(arg))
   %chName = GetName(ch)
   %obj = 3635

   //Check to see if they are a guard
   if(!HasObj(ch, obj))
     {
      DoWith(me)
      {
           [
             'I'm sorry but only Ice Haven guards may call upon The Holy Mother.
           ]
      }
     }

   if(HasObj(ch, obj))
   {
    if (arg == "HEAL")
    {
       if (GetGold(ch) >= prHeal)
       {
         DoWith(me)
         {
              [
                'The cost is $prHeal
                mpgold $chName -$prHeal
                cast '%arg' $chName
              ]
         }
       }
       else
       {
         DoWith(me)
         {
             [
                'Sorry $chName, but you do not have sufficient funds to use my services.
             ]
         }
       }
    } else if (%arg == "BLESS")
     {
       if (GetGold(ch) >= %prBless)
        {
          DoWith(me)
          {
               [
                 'The cost is %prBless
                 mpgold %chName - %prBless
                 cast '%arg' %chName
               ]
          }
    }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
     }
    } else if (%arg == "REFRESH")
     {
       if (GetGold(ch) >= %prBless)
       {
         DoWith(me)
         {
              [
                'The cost is %prRefresh
                mpgold %chName - %prRefresh
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "MANA")
    {
       if (GetGold(ch) >= %prMana)
       {
         DoWith(me)
         {
              [
                'The cost is %prMana
                mpgold %chName - %prMana
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "CURE LIGHT")
    {
       if (GetGold(ch) >= %prCureLight)
       {
         DoWith(me)
         {
              [
                'The cost is %prCureLight
                mpgold %chName - %prCureLight
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "CURE SERIOUS")
    {
       if (GetGold(ch) >= %prCureSerious)
       {
         DoWith(me)
         {
              [
                'The cost is %prCureSerious
                mpgold %chName - %prCureSerious
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "CURE CRITIC")
    {
       if (GetGold(ch) >= %prCureCritic)
       {
         DoWith(me)
         {
              [
                'The cost is %prCureCritic
                mpgold %chName - %prCureCritic
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "CURE BLIND")
    {
       if (GetGold(ch) >= %prCureBlind)
       {
         DoWith(me)
         {
              [
                'The cost is %prCureBlind
                mpgold %chName - %prCureBlind
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "REMOVE POISON")
    {
       if (GetGold(ch) >= %prRemovePoison)
       {
         DoWith(me)
         {
              [
                'The cost is %prRemovePoison
                mpgold %chName - %prRemovePoison
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "REMOVE CURSE")
    {
       if (GetGold(ch) >= %prRemoveCurse)
       {
         DoWith(me)
         {
              [
                'The cost is %prRemoveCurse
                mpgold %chName - %prRemoveCurse
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "ARMOR")
    {
       if (GetGold(ch) >= %prArmor)
       {
         DoWith(me)
         {
              [
                'The cost is %prArmor
                mpgold %chName - %prArmor
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "SANCTUARY")
    {
       if (GetGold(ch) >= %prSanctuary)
       {
         DoWith(me)
         {
              [
                'The cost is %prSanctuary
                mpgold %chName - %prSanctuary
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "SHIELD")
    {
       if (GetGold(ch) >= %prShield)
       {
         DoWith(me)
         {
              [
                'The cost is %prShield
                mpgold %chName - %prShield
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "STONE SKIN")
    {
       if (GetGold(ch) >= %prStoneSkin)
       {
         DoWith(me)
         {
              [
                'The cost is %prStoneSkin
                mpgold %chName - %prStoneSkin
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "STRENGTH")
    {
       if (GetGold(ch) >= %prStrength)
       {
         DoWith(me)
         {
              [
                'The cost is %prStrength
                mpgold %chName - %prStrength
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "BLESS")
    {
       if (GetGold(ch) >= %prBless)
       {
         DoWith(me)
         {
              [
                'The cost is %prBless
                mpgold %chName - %prBless
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "REGEN")
    {
       if (GetGold(ch) >= %prRegen)
       {
         DoWith(me)
         {
              [
                'The cost is %prRegen
                mpgold %chName - %prRegen
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "INVIS")
    {
       if (GetGold(ch) >= %prInvis)
       {
         DoWith(me)
         {
              [
                'The cost is %prInvis
                mpgold %chName - %prInvis
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "FLY")
    {
       if (GetGold(ch) >= %prFly)
       {
         DoWith(me)
         {
              [
                'The cost is %prFly
                mpgold %chName - %prFly
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "CREATE LIGHT")
    {
       if (GetGold(ch) >= %prCreateLight)
       {
         DoWith(me)
         {
              [
                'The cost is %prCreateLight
                mpgold %chName - %prCreateLight
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "SENSE AURA")
    {
       if (GetGold(ch) >= %prSenseAura)
       {
         DoWith(me)
         {
              [
                'The cost is %prSenseAura
                mpgold %chName - %prSenseAura
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "SENSE LIFE")
    {
       if (GetGold(ch) >= %prSenseLife)
       {
         DoWith(me)
         {
              [
                'The cost is %prSenseLife
                mpgold %chName - %prSenseLife
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "TRUESIGHT")
    {
       if (GetGold(ch) >= %prTrueSight)
       {
         DoWith(me)
         {
              [
                'The cost is %prTrueSight
                mpgold %chName - %prTrueSight
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "INFA")
    {
       if (GetGold(ch) >= %prInfra)
       {
         DoWith(me)
         {
              [
                'The cost is %prInfra
                mpgold %chName - %prInfra
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "DETECT INVIS")
    {
       if (GetGold(ch) >= %prDetectInvis)
       {
         DoWith(me)
         {
              [
                'The cost is %prDetectInvis
                mpgold %chName - %prDetectInvis
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "ENERGY WARD")
    {
       if (GetGold(ch) >= %prEnergyWard)
       {
         DoWith(me)
         {
              [
                'The cost is %prEnergyWard
                mpgold %chName - %prEnergyWard
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "ELECTRIC WARD")
    {
       if (GetGold(ch) >= %prElectricWard)
       {
         DoWith(me)
         {
              [
                'The cost is %prElectricWard
                mpgold %chName - %prElectricWard
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "ACID WARD")
    {
       if (GetGold(ch) >= %prAcidWard)
       {
         DoWith(me)
         {
              [
                'The cost is %prAcidWard
                mpgold %chName - %prAcidWard
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "FIRE WARD")
    {
       if (GetGold(ch) >= %prFireWard)
       {
         DoWith(me)
         {
              [
                'The cost is %prFireWard
                mpgold %chName - %prFireWard
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "COLD WARD")
    {
       if (GetGold(ch) >= %prColdWard)
       {
         DoWith(me)
         {
              [
                'The cost is %prColdWard
                mpgold %chName - %prColdWard
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "STONEFIST")
    {
       if (GetGold(ch) >= %prStoneFist)
       {
         DoWith(me)
         {
              [
                'The cost is %prStoneFist
                 mpgold %chName - %prStoneFist
                 cast '%arg' %chName
               ]
          }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "DISPEL")
    {
       if (GetGold(ch) >= %prDispel)
      {
         DoWith(me)
         {
              [
                'The cost is %prDispel
                mpgold %chName - %prDispel
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "SUCCOR")
    {
       if (GetGold(ch) >= %prSuccor)
       {
         DoWith(me)
         {
              [
                'The cost is %prSuccor
                mpgold %chName - %prSuccor
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
      }
     } else if (%arg == "KNOWLEDGE")
    {
       if (GetGold(ch) >= %prKnowledge)
       {
         DoWith(me)
         {
              [
                'The cost is %prKnowledge
                mpgold %chName - %prKnowledge
                cast '%arg' %chName
              ]
         }
     }
     else
     {
         DoWith(me)
         {
             [
                'Sorry %chName, but you do not have sufficient funds to use my services.
             ]
         }
     }
    }  else {
      DoWith(me) {
        [
            'The Holy Mother is not willing to cast that for you.
            'Please type LIST to get a listing of my available services.
            grin
        ]
      }
    }
   }
}//end of function I think
