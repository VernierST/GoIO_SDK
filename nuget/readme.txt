To update the nuget package for GoIO.SDK, update the package version # in the .nuspec file, run "nuget pack" on it, and then upload the package to nuget.org.  Easy!

You can install NuGet from http://docs.nuget.org/docs/start-here/installing-nuget

Using the command line tool, the command to create the package is:

nuget pack GoIO.SDK.nuspec

This will create a file named GoIO.SDK.<version>.nupkg, where version is the version # from the .nuspec file

P.S. don't forget to commit the change to the .nuspec file for the version #