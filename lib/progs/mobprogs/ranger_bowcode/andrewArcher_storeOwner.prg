>give_prog(broken-pine-branch, me=$t, ch=$n) {

   LoadObjOnChar(me, 8755)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 8756)
   LoadObjOnChar(me, 56)

   DoWith(me)
    {
     [
      'Ahhh I remember making bows from these as a child.
      emote walks over to a shaving table and in a blur of motions he twists the branch into a bow.
      smile
      'That was fun. I even managed to make some arrows from that. 
      put all.twisted-pine-arrow quiver
      give twisted-pine-bow $ch
      give quiver $ch 
      junk broken-pine-branch
     ]
    }
}
>give_prog(broken-oak-branch, me=$t, ch=$n) {

   LoadObjOnChar(me, 8757)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 8758)
   LoadObjOnChar(me, 56)

   DoWith(me)
    {
     [
      'I haven't made an oak bow in quite awhile, but it shouldn't be a problem.
      emote walks over to a shaving table and quickly twists the branch into a bow.
      ponder
      nod
      'That wasn't too bad, I even managed to make some arrows from that.
      put all.twisted-oak-arrow quiver
      give twisted-oak-bow $ch
      give quiver $ch
      junk broken-oak-branch
     ]
    }
}
>give_prog(severed-cherry-wood, me=$t, ch=$n) {

   LoadObjOnChar(me, 8759)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 8760)
   LoadObjOnChar(me, 56)

   DoWith(me)
    {
     [
      'Truth be, I have never made a bow from cherry wood before.
      ponder
      'I'll give it a go, but no promises.
      emote walks over to a shaving table and slowly shaves the wood into the shape of a bow.
      whistle
      wipe
      'That was too close.  There was a knot in the wood that almost splintered the wood!
      'The bow is still strong though, and I know that it'll shoot true.
      'I even managed to make some arrows from that.
      put all.twisted-cherry-arrow quiver
      give twisted-cherry-bow $ch
      give quiver $ch
      junk severed-cherry-wood
     ]
    }
}
>give_prog(bamboo-stock, me=$t, ch=$n) {

   DoWith(me)
    {
     [
      gasp
      'You found a bamboo stock?!  I never thought I would see one of these in my lifetime.
      emote inspects the bamboo stock.
      frown
      shake
      'I would love to make a bow from this, but I'm sorry to say that I just don't have the skill to.
      'You'll have to find the man whom trained me in the art of bow crafting.
      'He goes by the name of Silverbow.  The last I heard he had a shop somewhere near Mordilnia.
      give bamboo-stock $ch
     ]
    }
}
