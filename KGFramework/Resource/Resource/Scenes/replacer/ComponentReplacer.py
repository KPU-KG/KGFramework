import sys

fileName = sys.argv[1]
f = open(fileName, 'r')
data = f.read()
f.close()
src = [
    "AnimationComponent", "592399138",
    "CameraComponent", "2216269385",
    "GeometryComponent", "36578682",
    "LightComponent", "3191016132",
    "MaterialComponent", "1886146475",
    "ParticleEmitterComponent", "2383571556",
    "ShadowCasterComponent", "692022078",
    "Render3DComponent", "4070531333",
    "BoneTransformComponent", "3232426790",
]
dst = [
    "IAnimationComponent", "289973645",
    "ICameraComponent", "1509592448",
    "IGeometryComponent", "1162546763",
    "ILightComponent", "2391118483",
    "IMaterialComponent", "308235170",
    "IParticleEmitterComponent", "1905864175",
    "IShadowCasterComponent", "1896069075",
    "IRender3DComponent", "1184141904",
    "IBoneTransformComponent", "4139637",
]

for i in range(len(src)):
    data = data.replace(src[i], dst[i])

f = open(fileName, 'w')
f.write(data)
f.close()



