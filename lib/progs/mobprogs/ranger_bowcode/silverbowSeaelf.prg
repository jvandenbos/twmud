>give_prog(bamboo-stock, me=$t, ch=$n) {

   LoadObjOnChar(me, 8761)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 8762)
   LoadObjOnChar(me, 56)

   DoWith(me)
    {
     [
      frown
      'A Bamboo Bow aye... I haven't made one in awhile, but they don't call me Silverbow for nothing.
      ponder
      emote walks over to a huge tub of water, and by wetting the stock he slowly bends it into a bow.
      emote removes the bamboo from the tub and lets it dry over a bed of warm coals.
      ponder
      nod
      'Alright, that'll do. This is exceptional wood, I believe you'll be pleased.
      'I even managed to make some arrows from that stock.
      put all.sharp-bamboo-arrow quiver
      give curved-bamboo-bow $ch
      give quiver $ch
      junk bamboo-stock
     ]
    }
}
