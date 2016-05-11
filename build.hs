#! /usr/bin/env stack
-- stack --install-ghc runghc --package turtle

{-# LANGUAGE OverloadedStrings #-}

import           System.Info (os)
-- import Data.Text (pack)
-- import Debug.Trace (trace)
import           Turtle

buildDir :: Turtle.FilePath
buildDir = "BUILD"

parser::Parser (Bool, Bool)
parser = (,) <$> switch "full" 'F' "Perform full build"
             <*> switch "clean" 'C' "Clean"

main = do
    -- echo $ pack os
    (full, clean) <- options "Configure & build" parser
    when (full || clean) $ rmtree "BUILD"
    if clean then
        exit ExitSuccess
    else do
        mktree buildDir
        cd buildDir
        proc "cmake" (cmakeConfiguration "..") empty
        .&&. proc "cmake" ["--build", "."] empty

  where
    cmakeConfiguration dir =
        ["-DCMAKE_BUILD_TYPE=Release"]
        ++ case os of
            "darwin" -> ["-G", "Ninja", dir]
            "windows" -> ["-A", "x64", dir]
            "mingw32" -> ["-A", "x64", dir]
