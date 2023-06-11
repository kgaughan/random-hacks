#!/bin/sh -eux
export CGO_ENABLED=1
go build -C hello-plugin -trimpath -ldflags '-s -w' -buildmode=plugin .
go build -C runner -trimpath -ldflags '-s -w' .
mv hello-plugin/*.so runner
