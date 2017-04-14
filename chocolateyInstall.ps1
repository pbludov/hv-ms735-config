$packageName = 'hv-ms735-config'
$installerType = 'msi'
$url = 'https://github.com/pbludov/hv-ms735-config/releases/download/v1.0/hv-ms735-config.msi'
$silentArgs = '/Q'
$validExitCodes = @(0)


Install-ChocolateyPackage "$packageName" "$installerType" "$silentArgs" "$url"  -validExitCodes $validExitCodes
