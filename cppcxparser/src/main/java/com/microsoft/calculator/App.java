package com.microsoft.calculator;

import java.io.IOException;
import java.io.InputStream;

import com.microsoft.CPPCXLexer;
import com.microsoft.CPPCXParser;
import com.microsoft.CPPCXParser.TranslationUnitContext;

import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.ParseTreeWalker;

// property
// attribute

/**
 * Hello world!
 *
 */
public class App {
    public static void main(String[] args) {
        try {
            InputStream is = App.class.getResourceAsStream("/example.cpp");
            Lexer lexer = new CPPCXLexer(CharStreams.fromStream(is));
            TokenStream tokenStream = new CommonTokenStream(lexer);
            CPPCXParser parser = new CPPCXParser(tokenStream);

            TranslationUnitContext tu = parser.translationUnit();

            CxListener listener = new CxListener();
            ParseTreeWalker.DEFAULT.walk(listener, tu);

        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
