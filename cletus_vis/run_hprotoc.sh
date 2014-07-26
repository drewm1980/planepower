find ~/planepower -iname '*.proto' | xargs cp proto_defs/

for file in proto_defs:
    hprotoc --haskell_out=src -p Protos proto_defs/$(file)
