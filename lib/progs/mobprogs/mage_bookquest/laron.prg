>command_prog(cast 'knowledge', ch=$n, me=$t) {
GET_CLASS(ch)=$class
if(class==mage) {
 DoWith(me) {
  [
    'knowledge spell cast'
  ]
 }
}
}
